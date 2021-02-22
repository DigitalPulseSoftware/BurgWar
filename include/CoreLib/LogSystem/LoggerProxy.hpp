// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_LOGSYSTEM_LOGGERPROXY_HPP
#define BURGWAR_CORELIB_LOGSYSTEM_LOGGERPROXY_HPP

#include <CoreLib/Export.hpp>
#include <CoreLib/LogSystem/Logger.hpp>

namespace bw
{
	class BURGWAR_CORELIB_API LoggerProxy : public AbstractLogger
	{
		friend class LogContextPtr;

		public:
			inline LoggerProxy(const Logger& logParent);
			inline LoggerProxy(const LoggerProxy&) = default;
			LoggerProxy(LoggerProxy&&) noexcept = default;
			~LoggerProxy() = default;

			template<typename... Args> void LogFormat(const LogContext& context, Args&& ... args) const;

			void Log(const LogContext& context, std::string content) const override;
			void LogRaw(const LogContext& context, std::string_view content) const override;

			virtual void InitializeContext(LogContext& context) const;

			inline LogContextPtr PushContext() const;

			bool ShouldLog(const LogContext& context) const override;

			LoggerProxy& operator=(const LoggerProxy& logger) = delete;
			LoggerProxy& operator=(LoggerProxy&&) = delete;

		protected:
			virtual void OverrideContent(const LogContext& context, std::string& content) const;

			const Logger& m_logParent;
	};
}

#include <CoreLib/LogSystem/LoggerProxy.inl>

#endif
