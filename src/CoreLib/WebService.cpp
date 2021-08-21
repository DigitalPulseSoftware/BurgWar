// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/WebService.hpp>
#include <CoreLib/Version.hpp>
#include <CoreLib/LogSystem/Logger.hpp>
#include <Nazara/Core/ErrorFlags.hpp>
#include <CoreLib/CurlLibrary.hpp> //< include last because of curl/curl.h
#include <type_traits>

namespace bw
{
	WebService::WebService(const Logger& logger) :
	m_logger(logger)
	{
		assert(IsInitialized());
		m_curlMulti = s_curlLibrary->multi_init();
	}

	WebService::~WebService()
	{
		assert(IsInitialized());

		if (m_curlMulti)
		{
			for (auto&& [handle, request] : m_activeRequests)
				s_curlLibrary->multi_remove_handle(m_curlMulti, handle);

			s_curlLibrary->multi_cleanup(m_curlMulti);
		}
	}

	void WebService::AddRequest(std::unique_ptr<WebRequest>&& request)
	{
		assert(m_curlMulti);
		assert(request);

		CURL* handle = request->Prepare();

		curl_write_callback writeCallback = [](char* ptr, std::size_t size, std::size_t nmemb, void* userdata) -> std::size_t
		{
			WebRequest* request = static_cast<WebRequest*>(userdata);

			std::size_t totalSize = size * nmemb;
			if (!request->OnBodyResponse(ptr, totalSize))
				return 0;

			return totalSize;
		};

		s_curlLibrary->easy_setopt(handle, CURLOPT_WRITEFUNCTION, writeCallback);
		s_curlLibrary->easy_setopt(handle, CURLOPT_WRITEDATA, request.get());

		m_activeRequests.emplace(handle, std::move(request));

		s_curlLibrary->multi_add_handle(m_curlMulti, handle);
	}
	
	void WebService::Poll()
	{
		assert(m_curlMulti);

		int reportedActiveRequest;
		CURLMcode err = s_curlLibrary->multi_perform(m_curlMulti, &reportedActiveRequest);
		if (err != CURLM_OK)
		{
			bwLog(m_logger, LogLevel::Error, "[WebService] curl_multi_perform failed with {0}: {1}", err, s_curlLibrary->multi_strerror(err));
			return;
		}

		CURLMsg* m;
		do
		{
			int msgq;
			m = s_curlLibrary->multi_info_read(m_curlMulti, &msgq);
			if (m && (m->msg == CURLMSG_DONE))
			{
				CURL* handle = m->easy_handle;

				auto it = m_activeRequests.find(handle);
				assert(it != m_activeRequests.end());

				WebRequest& request = *it->second;

				if (m->data.result == CURLE_OK)
					request.TriggerCallback();
				else
					request.TriggerCallback(s_curlLibrary->easy_strerror(m->data.result));

				s_curlLibrary->multi_remove_handle(m_curlMulti, handle);

				m_activeRequests.erase(it);
			}
		}
		while (m);
	}

	bool WebService::Initialize(std::string* error)
	{
		assert(!IsInitialized());

		std::unique_ptr<CurlLibrary> libcurl = std::make_unique<CurlLibrary>();
		for (const char* libname : { "libcurl" NAZARA_DYNLIB_EXTENSION, "libcurl-d" NAZARA_DYNLIB_EXTENSION })
		{
			Nz::ErrorFlags errFlags(Nz::ErrorFlag_Silent);
			if (libcurl->library.Load(libname))
				break;
		}

		if (!libcurl->library.IsLoaded())
		{
			if (error)
				*error = "failed to load libcurl: " + libcurl->library.GetLastError().ToStdString();

			return false;
		}

		auto LoadFunction = [&](auto& funcPtr, const std::string& symbolName)
		{
			funcPtr = reinterpret_cast<std::decay_t<decltype(funcPtr)>>(libcurl->library.GetSymbol(symbolName));
			if (!funcPtr)
				throw std::runtime_error("failed to load " + symbolName + " from curl");
		};

		try
		{
#define BURGWAR_CURL_FUNCTION(F) LoadFunction(libcurl-> F, "curl_" #F);

#include <CoreLib/CurlFunctionList.hpp>
		}
		catch (const std::exception& e)
		{
			if (error)
				*error = e.what();

			return false;
		}

		if (libcurl->global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK)
			return false;

		curl_version_info_data* curlVersionData = libcurl->version_info(CURLVERSION_NOW);

		s_userAgent = "Burg'War/" +
		              std::to_string(GameMajorVersion) + "." + std::to_string(GameMinorVersion) + "." + std::to_string(GamePatchVersion) +
		              " WebService - curl/" + std::string(curlVersionData->version);

		s_curlLibrary = std::move(libcurl);
		return true;
	}

	void WebService::Uninitialize()
	{
		if (IsInitialized())
		{
			s_curlLibrary->global_cleanup();
			s_curlLibrary.reset();

			s_userAgent = std::string(); //< force buffer deletion
		}
	}

	std::string WebService::s_userAgent;
	std::unique_ptr<CurlLibrary> WebService::s_curlLibrary;
}
