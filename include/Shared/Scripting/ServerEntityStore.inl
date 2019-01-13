// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/Scripting/ServerEntityStore.hpp>
#include <cassert>

namespace bw
{
	inline ServerEntityStore::ServerEntityStore(std::shared_ptr<SharedGamemode> gamemode, std::shared_ptr<SharedScriptingContext> context) :
	SharedEntityStore(std::move(gamemode), std::move(context), true)
	{
	}
}
