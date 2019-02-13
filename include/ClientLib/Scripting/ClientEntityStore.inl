// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Scripting/ClientEntityStore.hpp>
#include <cassert>

namespace bw
{
	inline ClientEntityStore::ClientEntityStore(std::shared_ptr<SharedGamemode> gamemode, std::shared_ptr<SharedScriptingContext> context) :
	SharedEntityStore(std::move(gamemode), std::move(context), false)
	{
	}
}
