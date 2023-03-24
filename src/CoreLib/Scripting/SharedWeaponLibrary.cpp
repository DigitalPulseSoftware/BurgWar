// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/SharedWeaponLibrary.hpp>
#include <CoreLib/Scripting/AbstractScriptingLibrary.hpp>
#include <CoreLib/Components/CooldownComponent.hpp>
#include <CoreLib/Components/HealthComponent.hpp>
#include <CoreLib/Components/ScriptComponent.hpp>
#include <CoreLib/Components/WeaponComponent.hpp>
#include <CoreLib/Scripting/ScriptingUtils.hpp>
#include <Nazara/Utility/Components/NodeComponent.hpp>
#include <entt/entt.hpp>
#include <sol/sol.hpp>

namespace bw
{
	void SharedWeaponLibrary::RegisterLibrary(sol::table& elementMetatable)
	{
		RegisterSharedLibrary(elementMetatable);
	}

	void SharedWeaponLibrary::RegisterSharedLibrary(sol::table& elementMetatable)
	{
		elementMetatable["GetOwnerEntity"] = LuaFunction([](const sol::table& weaponTable) -> sol::object
		{
			entt::handle entity = AssertScriptEntity(weaponTable);

			entt::handle ownerEntity = entity.get<WeaponComponent>().GetOwner();
			if (!ownerEntity)
				return sol::nil;

			return ownerEntity.get<ScriptComponent>().GetTable();
		});

		elementMetatable["SetNextTriggerTime"] = LuaFunction([](const sol::table& weaponTable, Nz::UInt64 nextTriggerTime)
		{
			entt::handle entity = AssertScriptEntity(weaponTable);
			
			entity.get<CooldownComponent>().SetNextTriggerTime(Nz::Time::Milliseconds(nextTriggerTime));
		});
	}
}
