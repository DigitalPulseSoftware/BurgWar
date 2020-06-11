// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_PLAYERCOMMANDSTORE_HPP
#define BURGWAR_CORELIB_PLAYERCOMMANDSTORE_HPP

#include <CoreLib/CommandStore.hpp>

namespace bw
{
	class MatchClientSession;

	class PlayerCommandStore : public CommandStore<MatchClientSession>
	{
		public:
			PlayerCommandStore(const Logger& logger);
			~PlayerCommandStore() = default;
	};
}

#include <CoreLib/PlayerCommandStore.inl>

#endif
