// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_SHARED_CLIENTCOMMANDSTORE_HPP
#define BURGWAR_SHARED_CLIENTCOMMANDSTORE_HPP

#include <GameLibShared/CommandStore.hpp>

namespace bw
{
	class MatchClientSession;

	class PlayerCommandStore : public CommandStore<MatchClientSession>
	{
		public:
			PlayerCommandStore();
			~PlayerCommandStore() = default;
	};
}

#include <GameLibShared/PlayerCommandStore.inl>

#endif