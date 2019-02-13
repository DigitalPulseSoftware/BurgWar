// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Scripting/ClientWeaponStore.hpp>
#include <cassert>

namespace bw
{
	inline ClientWeaponStore::ClientWeaponStore(std::shared_ptr<SharedScriptingContext> context) :
	SharedWeaponStore(std::move(context), false)
	{
	}
}
