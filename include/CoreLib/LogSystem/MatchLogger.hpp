// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_LOGSYSTEM_MATCHLOGGER_HPP
#define BURGWAR_CORELIB_LOGSYSTEM_MATCHLOGGER_HPP

#include <CoreLib/LogSystem/Logger.hpp>
#include <CoreLib/LogSystem/MatchLogContext.hpp>

namespace bw
{
	class MatchLogger : public Logger
	{
		public:
			inline MatchLogger(SharedMatch& sharedMatch);
			inline MatchLogger(SharedMatch& sharedMatch, AbstractLogger& logger);
			~MatchLogger() = default;

			bool ShouldLog(const LogContext& context) const override;

		private:
			void OverrideContent(const LogContext& context, std::string& content) const override;

			LogLevel m_minimumLogLevel;
			MatchLogContext m_localMatchContext;
			SharedMatch& m_sharedMatch;
	};
}

#include <CoreLib/LogSystem/MatchLogger.inl>

#endif
