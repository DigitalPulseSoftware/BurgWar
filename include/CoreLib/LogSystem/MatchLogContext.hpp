// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_LOGSYSTEM_MATCHLOGCONTEXT_HPP
#define BURGWAR_CORELIB_LOGSYSTEM_MATCHLOGCONTEXT_HPP

#include <CoreLib/LogSystem/LogContext.hpp>

namespace bw
{
	class SharedMatch;

	struct MatchLogContext : LogContext
	{
		SharedMatch* match;
		Nz::UInt64 tick;
	};
}

#endif
