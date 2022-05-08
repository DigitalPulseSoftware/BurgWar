// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/ServerWeaponStore.hpp>
#include <CoreLib/BurgApp.hpp>
#include <CoreLib/TerrainLayer.hpp>
#include <CoreLib/Components/AnimationComponent.hpp>
#include <CoreLib/Components/MatchComponent.hpp>
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
	entt::entity ServerWeaponStore::InstantiateWeapon(TerrainLayer& layer, std::size_t weaponIndex, EntityId uniqueId, const PropertyValueMap& properties, entt::entity parent)
	{
		const auto& weaponClass = GetElement(weaponIndex);

		entt::entity weapon = CreateEntity(layer.GetWorld(), weaponClass, properties);
		weapon->AddComponent<MatchComponent>(layer.GetMatch(), layer.GetLayerIndex(), uniqueId);
		weapon->AddComponent<NetworkSyncComponent>(weaponClass->fullName, parent);

		SharedWeaponStore::InitializeWeapon(*weaponClass, weapon, parent);

		bwLog(GetLogger(), LogLevel::Debug, "Created weapon {} on layer {} of type {}", uniqueId, layer.GetLayerIndex(), GetElement(weaponIndex)->fullName);

		return weapon;
	}

	void ServerWeaponStore::InitializeElement(sol::main_table& elementTable, ScriptedWeapon& weapon)
	{
		SharedWeaponStore::InitializeElement(elementTable, weapon);
	}
}
