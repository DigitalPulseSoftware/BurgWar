// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/SharedWeaponLibrary.hpp>
#include <CoreLib/Scripting/AbstractScriptingLibrary.hpp>
#include <CoreLib/Components/HealthComponent.hpp>
#include <CoreLib/Components/ScriptComponent.hpp>
#include <CoreLib/Components/WeaponComponent.hpp>
#include <NDK/World.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <sol3/sol.hpp>

namespace bw
{
	void SharedWeaponLibrary::RegisterLibrary(sol::table& elementMetatable)
	{
		RegisterSharedLibrary(elementMetatable);
	}

	void SharedWeaponLibrary::RegisterSharedLibrary(sol::table& elementMetatable)
	{
		elementMetatable["GetOwnerEntity"] = [](const sol::table& weaponTable) -> sol::object
		{
			const Ndk::EntityHandle& entity = AssertScriptEntity(weaponTable);

			const Ndk::EntityHandle& ownerEntity = entity->GetComponent<WeaponComponent>().GetOwner();
			if (!ownerEntity)
				return sol::nil;

			return ownerEntity->GetComponent<ScriptComponent>().GetTable();
		};
	}
}
