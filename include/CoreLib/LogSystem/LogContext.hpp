// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_LOGSYSTEM_LOGCONTEXT_HPP
#define BURGWAR_CORELIB_LOGSYSTEM_LOGCONTEXT_HPP

#include <CoreLib/Export.hpp>
#include <CoreLib/LogSystem/Enums.hpp>

namespace bw
{
	struct BURGWAR_CORELIB_API LogContext
	{
		virtual ~LogContext();

		LogLevel level;
		LogSide side;
		float elapsedTime;
	};
}

#endif
