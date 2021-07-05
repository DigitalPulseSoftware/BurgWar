// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_CLIENTCOMMANDSTORE_HPP
#define BURGWAR_CLIENTLIB_CLIENTCOMMANDSTORE_HPP

#include <CoreLib/CommandStore.hpp>
#include <ClientLib/Export.hpp>

namespace bw
{
	class ClientSession;

	class BURGWAR_CLIENTLIB_API ClientCommandStore : public CommandStore<ClientSession*>
	{
		public:
			ClientCommandStore(const Logger& logger);
			~ClientCommandStore() = default;
	};
}

#include <ClientLib/ClientCommandStore.inl>

#endif
