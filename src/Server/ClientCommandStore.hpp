// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_SERVER_CLIENTCOMMANDSTORE_HPP
#define BURGWAR_SERVER_CLIENTCOMMANDSTORE_HPP

#include <Shared/CommandStore.hpp>

namespace bw
{
	class ClientSession;

	class ClientCommandStore : public CommandStore<ClientSession>
	{
		public:
			ClientCommandStore();
			~ClientCommandStore() = default;
	};
}

#include <Server/ClientCommandStore.inl>

#endif