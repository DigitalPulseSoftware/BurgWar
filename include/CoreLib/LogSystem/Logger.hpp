// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_LOGSYSTEM_LOGGER_HPP
#define BURGWAR_CORELIB_LOGSYSTEM_LOGGER_HPP

#include <CoreLib/LogSystem/AbstractLogger.hpp>
#include <CoreLib/LogSystem/Enums.hpp>
#include <CoreLib/LogSystem/LogContext.hpp>
#include <fmt/format.h>
#include <memory>
#include <vector>

#define bwLog(logObject, lvl, ...) do \
{ \
	bw::LogContext _bwLogContext; \
	_bwLogContext.level = lvl; \
	if ((logObject).ShouldLog(_bwLogContext)) \
		(logObject).LogFormat(_bwLogContext, __VA_ARGS__); \
} \
while (false)

namespace bw
{
	class LogSink;

	class Logger : public AbstractLogger
	{
		public:
			inline Logger();
			inline Logger(AbstractLogger& logParent);
			~Logger() = default;

			template<typename... Args> void LogFormat(LogContext& context, Args&& ... args);

			void Log(LogContext& context, std::string content) override;
			void LogRaw(LogContext& context, std::string_view content) override;

			inline void RegisterSink(std::shared_ptr<LogSink> sinkPtr);

			inline void SetMinimumLogLevel(LogLevel level);

			bool ShouldLog(const LogContext& context) const override;

		protected:
			virtual void OverrideContent(const LogContext& context, std::string& content);

		private:
			LogContext m_localContext;
			LogLevel m_minimumLogLevel;
			AbstractLogger* m_logParent;
			std::vector<std::shared_ptr<LogSink>> m_sinks;
	};
}

#include <CoreLib/LogSystem/Logger.inl>

#endif
