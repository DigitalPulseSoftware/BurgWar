// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_LOGSYSTEM_ABSTRACTLOGGER_HPP
#define BURGWAR_CORELIB_LOGSYSTEM_ABSTRACTLOGGER_HPP

#include <CoreLib/LogSystem/LogContext.hpp>
#include <string_view>

namespace bw
{
	class AbstractLogger
	{
		public:
			inline AbstractLogger(LogSide logSide);
			virtual ~AbstractLogger();

			inline LogSide GetSide() const;

			virtual void Log(const LogContext& context, std::string content) const = 0;
			virtual void LogRaw(const LogContext& context, std::string_view content) const = 0;

			virtual bool ShouldLog(const LogContext& context) const = 0;

		private:
			LogSide m_side;
	};
}

#include <CoreLib/LogSystem/AbstractLogger.inl>

#endif
