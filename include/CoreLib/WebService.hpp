// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_WEBREQUESTDISPATCHER_HPP
#define BURGWAR_CORELIB_WEBREQUESTDISPATCHER_HPP

#include <CoreLib/Export.hpp>
#include <CoreLib/WebRequest.hpp>
#include <Nazara/Core/MovablePtr.hpp>
#include <tsl/hopscotch_map.h>

using CURLM = void;

namespace bw
{
	class Logger;

	class BURGWAR_CORELIB_API WebService
	{
		friend class BurgApp;

		public:
			WebService(const Logger& logger);
			WebService(const WebService&) = delete;
			WebService(WebService&&) = default;
			~WebService();

			void AddRequest(std::unique_ptr<WebRequest>&& request);

			void Poll();

			WebService& operator=(const WebService&) = delete;
			WebService& operator=(WebService&&) = default;

			static inline const std::string& GetUserAgent();
			static inline bool IsInitialized();

		private:
			static bool Initialize();
			static void Uninitialize();

			const Logger& m_logger;
			Nz::MovablePtr<CURLM> m_curlMulti;
			tsl::hopscotch_map<CURL*, std::unique_ptr<WebRequest>> m_activeRequests;

			static bool s_isInitialized;
			static std::string s_userAgent;
	};
}

#include <CoreLib/WebService.inl>

#endif
