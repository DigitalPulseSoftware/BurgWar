// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/ServerEntityStore.hpp>
#include <cassert>

namespace bw
{
	inline ServerEntityStore::ServerEntityStore(AssetStore& assetStore, std::shared_ptr<ScriptingContext> context) :
	SharedEntityStore(assetStore, std::move(context), true)
	{
	}
}
