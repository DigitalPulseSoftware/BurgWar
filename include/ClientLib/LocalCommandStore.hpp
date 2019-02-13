// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_SERVERCOMMANDSTORE_HPP
#define BURGWAR_CLIENTLIB_SERVERCOMMANDSTORE_HPP

#include <CoreLib/CommandStore.hpp>

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

#include <ClientLib/LocalCommandStore.inl>

#endif