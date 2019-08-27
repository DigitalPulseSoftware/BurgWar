// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/ServerWeaponLibrary.hpp>
#include <CoreLib/AnimationStore.hpp>
#include <CoreLib/Match.hpp>
#include <CoreLib/Components/AnimationComponent.hpp>
#include <CoreLib/Components/HealthComponent.hpp>
#include <NDK/World.hpp>
#include <NDK/Components/PhysicsComponent2D.hpp>
#include <NDK/Systems/PhysicsSystem2D.hpp>
#include <sol3/sol.hpp>

namespace bw
{
	void ServerWeaponLibrary::RegisterLibrary(sol::table& elementMetatable)
	{
		SharedWeaponLibrary::RegisterLibrary(elementMetatable);

		RegisterServerLibrary(elementMetatable);
	}

	void ServerWeaponLibrary::RegisterServerLibrary(sol::table& elementMetatable)
	{
		auto DealDamage = [this](const sol::table& entityTable, const Nz::Vector2f& origin, Nz::UInt16 damage, Nz::Rectf damageZone, float pushbackForce = 0.f)
		{
			const Ndk::EntityHandle& entity = AssertScriptEntity(entityTable);
			Ndk::World* world = entity->GetWorld();
			assert(world);

			world->GetSystem<Ndk::PhysicsSystem2D>().RegionQuery(damageZone, 0, 0xFFFFFFFF, 0xFFFFFFFF, [&](const Ndk::EntityHandle& hitEntity)
			{
				if (hitEntity->HasComponent<HealthComponent>())
					hitEntity->GetComponent<HealthComponent>().Damage(damage, entity);

				if (hitEntity->HasComponent<Ndk::PhysicsComponent2D>())
				{
					Ndk::PhysicsComponent2D& hitEntityPhys = hitEntity->GetComponent<Ndk::PhysicsComponent2D>();
					hitEntityPhys.AddImpulse(Nz::Vector2f::Normalize(hitEntityPhys.GetMassCenter(Nz::CoordSys_Global) - origin) * pushbackForce);
				}
			});
		};

		elementMetatable["DealDamage"] = sol::overload(DealDamage,
			[=](const sol::table& entityTable, const Nz::Vector2f& origin, Nz::UInt16 damage, Nz::Rectf damageZone) { DealDamage(entityTable, origin, damage, damageZone); },
			[=](const sol::table& entityTable, const Nz::Vector2f& origin, Nz::UInt16 damage, Nz::Rectf damageZone, float pushbackForce) { DealDamage(entityTable, origin, damage, damageZone, pushbackForce); });

		elementMetatable["IsPlayingAnimation"] = [](const sol::table& weaponTable)
		{
			const Ndk::EntityHandle& entity = AssertScriptEntity(weaponTable);
			if (!entity->HasComponent<AnimationComponent>())
				return false;

			return entity->GetComponent<AnimationComponent>().IsPlaying();
		};

		elementMetatable["PlayAnim"] = [&](const sol::table& weaponTable, const std::string& animationName)
		{
			const Ndk::EntityHandle& entity = AssertScriptEntity(weaponTable);
			if (!entity->HasComponent<AnimationComponent>())
				throw std::runtime_error("Entity has no animations");

			auto& entityAnimation = entity->GetComponent<AnimationComponent>();
			const auto& animationStore = entityAnimation.GetAnimationStore();

			if (std::size_t animId = animationStore->FindAnimationByName(animationName); animId != animationStore->InvalidId)
				entityAnimation.Play(animId, m_match.GetCurrentTime());
			else
				throw std::runtime_error("Entity has no animation \"" + animationName + "\"");
		};

		auto shootFunc = [](const sol::table& weaponTable, Nz::Vector2f startPos, Nz::Vector2f direction, Nz::UInt16 damage, float pushbackForce = 0.f)
		{
			const Ndk::EntityHandle& entity = AssertScriptEntity(weaponTable);
			Ndk::World* world = entity->GetWorld();
			assert(world);

			auto& physSystem = world->GetSystem<Ndk::PhysicsSystem2D>();

			Ndk::PhysicsSystem2D::RaycastHit hitInfo;

			if (physSystem.RaycastQueryFirst(startPos, startPos + direction * 1000.f, 1.f, 0, 0xFFFFFFFF, 0xFFFFFFFF, &hitInfo))
			{
				const Ndk::EntityHandle& hitEntity = hitInfo.body;

				if (hitEntity->HasComponent<HealthComponent>())
					hitEntity->GetComponent<HealthComponent>().Damage(damage, entity);

				if (hitEntity->HasComponent<Ndk::PhysicsComponent2D>())
				{
					Ndk::PhysicsComponent2D& hitEntityPhys = hitEntity->GetComponent<Ndk::PhysicsComponent2D>();
					hitEntityPhys.AddImpulse(Nz::Vector2f::Normalize(hitInfo.hitPos - startPos) * pushbackForce);
				}
			}
		};

		elementMetatable["Shoot"] = sol::overload(shootFunc, [=](const sol::table& weaponTable, Nz::Vector2f startPos, Nz::Vector2f direction, Nz::UInt16 damage) { shootFunc(weaponTable, startPos, direction, damage); });
	}
}
