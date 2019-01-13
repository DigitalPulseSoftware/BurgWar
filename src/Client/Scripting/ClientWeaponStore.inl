// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/Scripting/ClientWeaponStore.hpp>
#include <cassert>

namespace bw
{
	inline ClientWeaponStore::ClientWeaponStore(std::shared_ptr<SharedGamemode> gamemode, std::shared_ptr<SharedScriptingContext> context) :
	SharedWeaponStore(std::move(gamemode), std::move(context), false)
	{
	}
}
