// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENT_SERVERCOMMANDSTORE_HPP
#define BURGWAR_CLIENT_SERVERCOMMANDSTORE_HPP

#include <GameLibShared/CommandStore.hpp>

namespace bw
{
	class ClientSession;

	class LocalCommandStore : public CommandStore<ClientSession*>
	{
		public:
			LocalCommandStore();
			~LocalCommandStore() = default;
	};
}

#include <Client/LocalCommandStore.inl>

#endif