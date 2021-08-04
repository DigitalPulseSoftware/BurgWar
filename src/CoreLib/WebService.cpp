// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/WebService.hpp>
#include <CoreLib/LogSystem/Logger.hpp>
#include <curl/curl.h>

namespace bw
{
	WebService::WebService(const Logger& logger) :
	m_logger(logger)
	{
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
			request->AppendBodyResponse(ptr, totalSize);

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
				{
					long responseCode;
					curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &responseCode);

					request.TriggerCallback(static_cast<unsigned int>(responseCode));
				}
				else
					request.TriggerCallback(curl_easy_strerror(m->data.result));

				curl_multi_remove_handle(m_curlMulti, handle);

				m_activeRequests.erase(it);
			}
		}
		while (m);
	}
}
