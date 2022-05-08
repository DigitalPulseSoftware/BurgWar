// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_LOGSYSTEM_ENTITYLOGCONTEXT_HPP
#define BURGWAR_CORELIB_LOGSYSTEM_ENTITYLOGCONTEXT_HPP

#include <CoreLib/Export.hpp>
#include <CoreLib/LogSystem/MatchLogContext.hpp>
#include <entt/entt.hpp>

namespace bw
{
	struct EntityLogContext : MatchLogContext
	{
		entt::entity entity;
	};
}

#endif
