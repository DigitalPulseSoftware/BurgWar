// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/Scripting/ServerWeaponStore.hpp>
#include <Shared/BurgApp.hpp>
#include <Shared/Components/AnimationComponent.hpp>
#include <Shared/Components/NetworkSyncComponent.hpp>
#include <Shared/Components/ScriptComponent.hpp>
#include <Shared/Systems/NetworkSyncSystem.hpp>
#include <Shared/Systems/PlayerMovementSystem.hpp>
#include <Nazara/Math/Rect.hpp>
#include <Nazara/Utility/Image.hpp>
#include <NDK/Components.hpp>
#include <NDK/Systems.hpp>
#include <NDK/World.hpp>
#include <iostream>

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

	void ServerWeaponStore::InitializeElementTable(sol::table& elementTable)
	{
		SharedWeaponStore::InitializeElementTable(elementTable);

		elementTable["DealDamage"] = [](const sol::table& weaponTable, Nz::UInt16 damage, Nz::Rectf damageZone)
		{
			const Ndk::EntityHandle& entity = weaponTable["Entity"];
			Ndk::World* world = entity->GetWorld();
			assert(world);

			auto& nodeComponent = entity->GetComponent<Ndk::NodeComponent>();

			Nz::Vector2f pos = Nz::Vector2f(nodeComponent.GetPosition());

			std::vector<Ndk::EntityHandle> hitEntities;
			world->GetSystem<Ndk::PhysicsSystem2D>().RegionQuery(damageZone, 0, 0xFFFFFFFF, 0xFFFFFFFF, &hitEntities);

			for (const Ndk::EntityHandle& hitEntity : hitEntities)
			{
				if (hitEntity->HasComponent<HealthComponent>())
					hitEntity->GetComponent<HealthComponent>().Damage(damage, entity);

				std::cout << hitEntity << std::endl;
			}
		};

		elementTable["IsPlayingAnimation"] = [](const sol::table& weaponTable)
		{
			const Ndk::EntityHandle& entity = weaponTable["Entity"];
			if (!entity->HasComponent<AnimationComponent>())
				return false;

			return entity->GetComponent<AnimationComponent>().IsPlaying();
		};

		elementTable["PlayAnim"] = [&](const sol::table& weaponTable, const std::string& animationName)
		{
			const Ndk::EntityHandle& entity = weaponTable["Entity"];
			if (!entity->HasComponent<AnimationComponent>())
				throw std::runtime_error("Entity has no animations");

			auto& entityAnimation = entity->GetComponent<AnimationComponent>();
			const auto& animationStore = entityAnimation.GetAnimationStore();

			if (std::size_t animId = animationStore->FindAnimationByName(animationName); animId != animationStore->InvalidId)
				entityAnimation.Play(animId, m_application.GetAppTime());
			else
				throw std::runtime_error("Entity has no animation \"" + animationName + "\"");
		};
	}

	void ServerWeaponStore::InitializeElement(sol::table& elementTable, ScriptedWeapon& weapon)
	{
		SharedWeaponStore::InitializeElement(elementTable, weapon);
	}
}
