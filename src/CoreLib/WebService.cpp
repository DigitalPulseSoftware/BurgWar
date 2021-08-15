// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/WebService.hpp>
#include <CoreLib/Version.hpp>
#include <CoreLib/LogSystem/Logger.hpp>
#include <curl/curl.h>

namespace bw
{
	WebService::WebService(const Logger& logger) :
	m_logger(logger)
	{
		assert(s_isInitialized);
		m_curlMulti = curl_multi_init();
	}

	WebService::~WebService()
	{
		if (m_curlMulti)
		{
			for (auto&& [handle, request] : m_activeRequests)
				curl_multi_remove_handle(m_curlMulti, handle);

			curl_multi_cleanup(m_curlMulti);
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

		curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, writeCallback);
		curl_easy_setopt(handle, CURLOPT_WRITEDATA, request.get());

		m_activeRequests.emplace(handle, std::move(request));

		curl_multi_add_handle(m_curlMulti, handle);
	}
	
	void WebService::Poll()
	{
		assert(m_curlMulti);

		int reportedActiveRequest;
		CURLMcode err = curl_multi_perform(m_curlMulti, &reportedActiveRequest);
		if (err != CURLM_OK)
		{
			bwLog(m_logger, LogLevel::Error, "[WebService] curl_multi_perform failed with {0}: {1}", err, curl_multi_strerror(err));
			return;
		}

		CURLMsg* m;
		do
		{
			int msgq;
			m = curl_multi_info_read(m_curlMulti, &msgq);
			if (m && (m->msg == CURLMSG_DONE))
			{
				CURL* handle = m->easy_handle;

				auto it = m_activeRequests.find(handle);
				assert(it != m_activeRequests.end());

				WebRequest& request = *it->second;

				if (m->data.result == CURLE_OK)
					request.TriggerCallback();
				else
					request.TriggerCallback(curl_easy_strerror(m->data.result));

				curl_multi_remove_handle(m_curlMulti, handle);

				m_activeRequests.erase(it);
			}
		}
		while (m);
	}

	bool WebService::Initialize()
	{
		assert(!s_isInitialized);
		if (curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK)
			return false;

		curl_version_info_data* curlVersionData = curl_version_info(CURLVERSION_NOW);

		s_isInitialized = true;
		s_userAgent = "Burg'War/" 
		              NazaraStringifyMacro(BURGWAR_VERSION_MAJOR) "." NazaraStringifyMacro(BURGWAR_VERSION_MINOR) "." NazaraStringifyMacro(BURGWAR_VERSION_PATCH) 
		              " WebService - curl/" + std::string(curlVersionData->version);

		return true;
	}

	void WebService::Uninitialize()
	{
		if (s_isInitialized)
		{
			curl_global_cleanup();
			s_isInitialized = false;
			s_userAgent = std::string(); //< force buffer deletion
		}
	}

	bool WebService::s_isInitialized = false;
	std::string WebService::s_userAgent;
}
