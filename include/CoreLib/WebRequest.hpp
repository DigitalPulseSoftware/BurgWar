// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_WEBREQUEST_HPP
#define BURGWAR_CORELIB_WEBREQUEST_HPP

#include <CoreLib/Export.hpp>
#include <Nazara/Core/MovablePtr.hpp>
#include <tsl/hopscotch_map.h>
#include <functional>
#include <optional>
#include <string>

using CURL = void;
struct curl_slist;

namespace bw
{
	class BURGWAR_CORELIB_API WebRequest
	{
		friend class WebService;

		public:
			using Callback = std::function<void(std::optional<unsigned int> code, std::string body)>;

			WebRequest();
			WebRequest(const WebRequest&) = delete;
			WebRequest(WebRequest&&) = default;
			~WebRequest();

			inline void SetCallback(Callback callback);
			inline void SetHeader(std::string header, std::string value);
			void SetJSonContent(const std::string_view& encodedJSon);
			void SetURL(const std::string& url);

			void SetupGet();
			void SetupPost();

			WebRequest& operator=(const WebRequest&) = delete;
			WebRequest& operator=(WebRequest&&) = default;

			static std::unique_ptr<WebRequest> Get(const std::string& url, Callback callback = nullptr);
			static std::unique_ptr<WebRequest> Post(const std::string& url, Callback callback = nullptr);

		private:
			inline void AppendBodyResponse(const char* data, std::size_t length);
			CURL* Prepare();
			inline void TriggerCallback(unsigned int code);
			inline void TriggerCallback(std::string errorMessage);

			Nz::MovablePtr<CURL> m_curlHandle;
			Nz::MovablePtr<curl_slist> m_headerList;
			tsl::hopscotch_map<std::string, std::string> m_headers;
			Callback m_callback;
			std::string m_responseBody;
	};
}

#include <CoreLib/WebRequest.inl>

#endif
