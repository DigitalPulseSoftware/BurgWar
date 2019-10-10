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
			virtual ~AbstractLogger();

			virtual void Log(LogContext& context, std::string content) = 0;
			virtual void LogRaw(LogContext& context, std::string_view content) = 0;

			virtual bool ShouldLog(const LogContext& context) const = 0;
	};
}

#endif
