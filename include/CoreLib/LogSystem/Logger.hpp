// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_LOGSYSTEM_LOGGER_HPP
#define BURGWAR_CORELIB_LOGSYSTEM_LOGGER_HPP

#include <CoreLib/LogSystem/Enums.hpp>
#include <CoreLib/LogSystem/LogContext.hpp>
#include <CoreLib/LogSystem/LogSink.hpp>
#include <fmt/format.h>
#include <memory>
#include <vector>

#define bwLog(logObject, lvl, ...) do \
{ \
	bw::LogContext _bwLogContext; \
	_bwLogContext.level = lvl; \
	if ((logObject).ShouldLog(_bwLogContext)) \
		(logObject).Log(_bwLogContext, __VA_ARGS__); \
} \
while (false) \

namespace bw
{
	struct NoLogger;

	template<typename T, typename Context = LogContext>
	class Logger
	{
		public:
			Logger();
			Logger(T& logParent);
			~Logger() = default;

			template<typename... Args> void Log(Context& context, Args&& ... args);
			void LogRaw(Context& context, std::string_view content);

			inline void RegisterSink(std::shared_ptr<LogSink<Context>> sinkPtr);

			void SetMinimumLogLevel(LogLevel level);

			bool ShouldLog(const Context& context) const;

		private:
			LogLevel m_minimumLogLevel;
			T* m_logParent;
			std::vector<std::shared_ptr<LogSink<Context>>> m_sinks;
	};
}

#include <CoreLib/LogSystem/Logger.inl>

#endif
