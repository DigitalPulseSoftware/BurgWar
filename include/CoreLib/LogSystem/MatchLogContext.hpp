// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_LOGSYSTEM_MATCHLOGCONTEXT_HPP
#define BURGWAR_CORELIB_LOGSYSTEM_MATCHLOGCONTEXT_HPP

#include <CoreLib/LogSystem/LogContext.hpp>

namespace bw
{
	class Match;

	struct MatchLogContext : LogContext
	{
		Match* match;
	};
}

#endif
