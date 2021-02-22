// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_LOCALCOMMANDSTORE_HPP
#define BURGWAR_CLIENTLIB_LOCALCOMMANDSTORE_HPP

#include <CoreLib/CommandStore.hpp>
#include <ClientLib/Export.hpp>

namespace bw
{
	class ClientSession;

	class BURGWAR_CLIENTLIB_API LocalCommandStore : public CommandStore<ClientSession*>
	{
		public:
			LocalCommandStore(const Logger& logger);
			~LocalCommandStore() = default;
	};
}

#include <ClientLib/LocalCommandStore.inl>

#endif
