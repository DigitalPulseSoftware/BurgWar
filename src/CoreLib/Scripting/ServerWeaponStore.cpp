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

		auto DealDamage = [](const sol::table& weaponTable, Nz::UInt16 damage, Nz::Rectf damageZone, float pushbackForce = 0.f)
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

				if (hitEntity->HasComponent<Ndk::PhysicsComponent2D>())
				{
					Ndk::PhysicsComponent2D& hitEntityPhys = hitEntity->GetComponent<Ndk::PhysicsComponent2D>();
					hitEntityPhys.AddImpulse(Nz::Vector2f::Normalize(hitEntityPhys.GetMassCenter(Nz::CoordSys_Global) - pos) * pushbackForce);
				}
			}
		};

		elementTable["DealDamage"] = sol::overload(DealDamage,
			[=](const sol::table& weaponTable, Nz::UInt16 damage, Nz::Rectf damageZone) { DealDamage(weaponTable, damage, damageZone); });

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
