// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_WEBREQUEST_HPP
#define BURGWAR_CORELIB_WEBREQUEST_HPP

#include <CoreLib/Export.hpp>
#include <CoreLib/WebRequestResult.hpp>
#include <Nazara/Core/MovablePtr.hpp>
#include <Nazara/Network/Enums.hpp>
#include <tsl/hopscotch_map.h>
#include <functional>
#include <string>

struct curl_slist;

namespace bw
{
	class BURGWAR_CORELIB_API WebRequest
	{
		friend class WebService;

		public:
			using DataCallback = std::function<bool(const void* data, std::size_t length)>;
			using ResultCallback = std::function<void(WebRequestResult&& result)>;

			WebRequest();
			WebRequest(const WebRequest&) = delete;
			WebRequest(WebRequest&&) = default;
			~WebRequest();

			void ForceProtocol(Nz::NetProtocol protocol);

			inline void SetDataCallback(DataCallback callback);
			inline void SetHeader(std::string header, std::string value);
			void SetJSonContent(const std::string_view& encodedJSon);
			void SetMaximumFileSize(Nz::UInt64 maxFileSize);
			inline void SetResultCallback(ResultCallback callback);
			void SetServiceName(const std::string_view& serviceName);
			void SetURL(const std::string& url);

			void SetupGet();
			void SetupPost();

			WebRequest& operator=(const WebRequest&) = delete;
			WebRequest& operator=(WebRequest&&) = default;

			static std::unique_ptr<WebRequest> Get(const std::string& url, ResultCallback callback = nullptr);
			static std::unique_ptr<WebRequest> Post(const std::string& url, ResultCallback callback = nullptr);

		private:
			inline bool OnBodyResponse(const char* data, std::size_t length);
			CURL* Prepare();
			inline void TriggerCallback();
			inline void TriggerCallback(std::string errorMessage);

			Nz::MovablePtr<CURL> m_curlHandle;
			Nz::MovablePtr<curl_slist> m_headerList;
			std::string m_responseBody;
			tsl::hopscotch_map<std::string, std::string> m_headers;
			DataCallback m_dataCallback;
			ResultCallback m_resultCallback;
			bool m_isUserAgentSet;
	};
}

#include <CoreLib/WebRequest.inl>

#endif
