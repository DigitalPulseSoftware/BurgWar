// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/ServerWeaponStore.hpp>
#include <cassert>

namespace bw
{
	inline ServerWeaponStore::ServerWeaponStore(const Logger& logger, std::shared_ptr<ScriptingContext> context) :
	SharedWeaponStore(logger, std::move(context), true)
	{
	}
}
