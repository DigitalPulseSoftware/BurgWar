// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_SHARED_CLIENTCOMMANDSTORE_HPP
#define BURGWAR_SHARED_CLIENTCOMMANDSTORE_HPP

#include <Shared/CommandStore.hpp>

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

#include <Shared/PlayerCommandStore.inl>

#endif