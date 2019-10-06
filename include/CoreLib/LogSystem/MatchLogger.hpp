// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_LOGSYSTEM_MATCHLOGGER_HPP
#define BURGWAR_CORELIB_LOGSYSTEM_MATCHLOGGER_HPP

#include <CoreLib/LogSystem/Logger.hpp>
#include <CoreLib/LogSystem/MatchLogContext.hpp>

#define bwLog(logObject, lvl, ...) do \
{ \
	bw::LogContext _bwLogContext; \
	_bwLogContext.level = lvl; \
	if ((logObject).ShouldLog(_bwLogContext)) \
		(logObject).Log(_bwLogContext, __VA_ARGS__); \
} \
while (false)

#define bwMatchLog(logObject, match_, lvl, ...) do \
{ \
	bw::MatchLogContext _bwLogContext; \
	_bwLogContext.level = lvl; \
	_bwLogContext.match = match_; \
	if ((logObject).ShouldLog(_bwLogContext)) \
		(logObject).Log(_bwLogContext, __VA_ARGS__); \
} \
while (false)

namespace bw
{
	template<typename T, typename Context = MatchLogContext>
	class MatchLogger : public Logger<T, Context>
	{
		public:
			using Logger::Logger;
			~MatchLogger() = default;

			bool ShouldLog(const Context& context) const override;

		private:
			void OverrideContent(const Context& context, std::string& content) override;

			LogLevel m_minimumLogLevel;
			T* m_logParent;
			std::vector<std::shared_ptr<LogSink<Context>>> m_sinks;
	};
}

#include <CoreLib/LogSystem/MatchLogger.inl>

#endif
