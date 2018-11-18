// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/Scripting/ServerWeaponStore.hpp>
#include <NDK/Components.hpp>
#include <Nazara/Utility/Image.hpp>
#include <Shared/Components/NetworkSyncComponent.hpp>
#include <Shared/Components/PlayerControlledComponent.hpp>
#include <Shared/Components/PlayerMovementComponent.hpp>
#include <Shared/Systems/NetworkSyncSystem.hpp>
#include <Shared/Systems/PlayerControlledSystem.hpp>
#include <Shared/Systems/PlayerMovementSystem.hpp>
#include <iostream>

namespace bw
{
	ServerWeaponStore::ServerWeaponStore(std::shared_ptr<SharedScriptingContext> context) :
	ScriptStore(std::move(context))
	{
		SetElementTypeName("weapon");
		SetTableName("WEAPON");
	}

	const Ndk::EntityHandle& ServerWeaponStore::InstantiateWeapon(Ndk::World& world, std::size_t weaponIndex, const Ndk::EntityHandle& parent)
	{
		auto& weaponClass = GetElement(weaponIndex);

		Nz::LuaState& state = GetLuaState();

		const Ndk::EntityHandle& weapon = world.CreateEntity();
		weapon->AddComponent<Ndk::NodeComponent>().SetParent(parent);
		weapon->AddComponent<NetworkSyncComponent>(weaponClass.fullName, parent);

		return weapon;
	}

	void ServerWeaponStore::InitializeElementTable(Nz::LuaState& state)
	{
	}

	void ServerWeaponStore::InitializeElement(Nz::LuaState& state, ScriptedWeapon& weapon)
	{
		// Actually server doesn't care about those
		weapon.scale = state.CheckField<float>("Scale");
		weapon.spriteName = state.CheckField<std::string>("Sprite");
	}
}
