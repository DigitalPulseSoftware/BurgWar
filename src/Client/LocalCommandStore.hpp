// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENT_SERVERCOMMANDSTORE_HPP
#define BURGWAR_CLIENT_SERVERCOMMANDSTORE_HPP

#include <Shared/CommandStore.hpp>

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