// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/ServerWeaponStore.hpp>
#include <CoreLib/BurgApp.hpp>
#include <CoreLib/Components/AnimationComponent.hpp>
#include <CoreLib/Components/NetworkSyncComponent.hpp>
#include <CoreLib/Components/ScriptComponent.hpp>
#include <CoreLib/Systems/NetworkSyncSystem.hpp>
#include <CoreLib/Systems/PlayerMovementSystem.hpp>
#include <Nazara/Math/Rect.hpp>
#include <Nazara/Utility/Image.hpp>
#include <NDK/Components.hpp>
#include <NDK/Systems.hpp>
#include <NDK/World.hpp>

namespace bw
{
	const Ndk::EntityHandle& ServerWeaponStore::InstantiateWeapon(Ndk::World& world, std::size_t weaponIndex, const EntityProperties& properties, const Ndk::EntityHandle& parent)
	{
		const auto& weaponClass = GetElement(weaponIndex);

		const Ndk::EntityHandle& weapon = CreateEntity(world, weaponClass, properties);
		weapon->AddComponent<NetworkSyncComponent>(weaponClass->fullName, parent);

		SharedWeaponStore::InitializeWeapon(*weaponClass, weapon, parent);

		return weapon;
	}

	void ServerWeaponStore::InitializeElement(sol::table& elementTable, ScriptedWeapon& weapon)
	{
		SharedWeaponStore::InitializeElement(elementTable, weapon);
	}
}
