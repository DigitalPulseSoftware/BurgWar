// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_LOGSYSTEM_LOGGER_HPP
#define BURGWAR_CORELIB_LOGSYSTEM_LOGGER_HPP

#include <CoreLib/Export.hpp>
#include <CoreLib/LogSystem/AbstractLogger.hpp>
#include <CoreLib/LogSystem/Enums.hpp>
#include <CoreLib/LogSystem/LogContext.hpp>
#include <CoreLib/LogSystem/LogContextPtr.hpp>
#include <NazaraUtils/MovablePtr.hpp>
#include <NazaraUtils/MemoryPool.hpp>
#include <fmt/format.h>
#include <fmt/std.h>
#include <memory>
#include <vector>

#define bwLog(logObject, lvl, ...) do \
{ \
	auto _bwLogContext = (logObject).PushContext(); \
	_bwLogContext->level = lvl; \
	if ((logObject).ShouldLog(*_bwLogContext)) \
		(logObject).LogFormat(*_bwLogContext, __VA_ARGS__); \
} \
while (false)

namespace bw
{
	class BurgAppComponent;
	class LogSink;

	class BURGWAR_CORELIB_API Logger : public AbstractLogger
	{
		friend class LogContextPtr;
		friend class LoggerProxy;

		public:
			inline Logger(BurgAppComponent& app, LogSide logSide);
			inline Logger(BurgAppComponent& app, LogSide logSide, const AbstractLogger& logParent);
			Logger(const Logger&) = delete;
			Logger(Logger&&) noexcept = default;
			~Logger() = default;

			template<typename... Args> void LogFormat(const LogContext& context, fmt::format_string<Args...> fmt, Args&& ... args) const;

			void Log(const LogContext& context, std::string content) const override;
			void LogRaw(const LogContext& context, std::string_view content) const override;

			inline LogContextPtr PushContext() const;

			inline void RegisterSink(std::shared_ptr<LogSink> sinkPtr);

			inline void SetMinimumLogLevel(LogLevel level);

			bool ShouldLog(const LogContext& context) const override;

			Logger& operator=(const Logger& logger) = delete;
			Logger& operator=(Logger&&) = delete;

		protected:
			template<typename T> T* AllocateContext() const;
			template<typename T> LogContextPtr PushCustomContext() const;

			virtual void InitializeContext(LogContext& context) const;
			virtual LogContext* NewContext() const;
			virtual void OverrideContent(const LogContext& context, std::string& content) const;

		private:
			void FreeContext(LogContext* context) const;

			BurgAppComponent& m_app;
			LogLevel m_minimumLogLevel;
			Nz::MovablePtr<const AbstractLogger> m_logParent;
			std::vector<std::shared_ptr<LogSink>> m_sinks;
	};
}

#include <CoreLib/LogSystem/Logger.inl>

#endif
