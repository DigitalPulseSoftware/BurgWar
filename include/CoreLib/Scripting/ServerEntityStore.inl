// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/ServerEntityStore.hpp>
#include <cassert>

namespace bw
{
	inline ServerEntityStore::ServerEntityStore(std::shared_ptr<SharedGamemode> gamemode, std::shared_ptr<SharedScriptingContext> context) :
	SharedEntityStore(std::move(gamemode), std::move(context), true)
	{
	}
}
