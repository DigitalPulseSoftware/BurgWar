// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SCRIPTING_GAMEMODEEVENTCONNECTION_HPP
#define BURGWAR_CORELIB_SCRIPTING_GAMEMODEEVENTCONNECTION_HPP

#include <CoreLib/Scripting/GamemodeEvents.hpp>
#include <cstddef>
#include <variant>

namespace bw
{
	struct GamemodeEventConnection
	{
		std::variant<GamemodeEvent, std::size_t> event;
		std::size_t callbackId;
	};
}

#endif
