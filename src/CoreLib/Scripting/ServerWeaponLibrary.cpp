// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/ServerWeaponLibrary.hpp>
#include <CoreLib/AnimationStore.hpp>
#include <CoreLib/Match.hpp>
#include <CoreLib/Components/AnimationComponent.hpp>
#include <CoreLib/Components/HealthComponent.hpp>
#include <CoreLib/Scripting/ScriptingUtils.hpp>
#include <sol/sol.hpp>

namespace bw
{
	void ServerWeaponLibrary::RegisterLibrary(sol::table& elementMetatable)
	{
		SharedWeaponLibrary::RegisterLibrary(elementMetatable);

		RegisterServerLibrary(elementMetatable);
	}

	void ServerWeaponLibrary::RegisterServerLibrary(sol::table& elementMetatable)
	{
		elementMetatable["IsPlayingAnimation"] = LuaFunction([](const sol::table& weaponTable)
		{
			entt::handle entity = AssertScriptEntity(weaponTable);

			AnimationComponent* entityAnimation = entity.try_get<AnimationComponent>();
			if (!entityAnimation)
				return false;

			return entityAnimation->IsPlaying();
		});

		elementMetatable["PlayAnim"] = LuaFunction([&](const sol::table& weaponTable, const std::string& animationName)
		{
			entt::handle entity = AssertScriptEntity(weaponTable);

			AnimationComponent* entityAnimation = entity.try_get<AnimationComponent>();
			if (!entityAnimation)
				throw std::runtime_error("Entity has no animations");

			const auto& animationStore = entityAnimation->GetAnimationStore();

			if (std::size_t animId = animationStore->FindAnimationByName(animationName); animId != animationStore->InvalidId)
				entityAnimation->Play(animId, m_match.GetCurrentTime());
			else
				throw std::runtime_error("Entity has no animation \"" + animationName + "\"");
		});

		auto shootFunc = [](const sol::table& weaponTable, Nz::Vector2f startPos, Nz::Vector2f direction, Nz::UInt16 damage, float pushbackForce = 0.f)
		{
			entt::handle entity = AssertScriptEntity(weaponTable);
			/*entt::registry* world = entity->GetWorld();
			assert(world);

			auto& physSystem = world->GetSystem<Ndk::PhysicsSystem2D>();

			Ndk::PhysicsSystem2D::RaycastHit hitInfo;

			if (physSystem.RaycastQueryFirst(startPos, startPos + direction * 1000.f, 1.f, 0, 0xFFFFFFFF, 0xFFFFFFFF, &hitInfo))
			{
				entt::entity hitEntity = hitInfo.body;

				if (hitEntity->HasComponent<HealthComponent>())
					hitEntity->GetComponent<HealthComponent>().Damage(damage, entity);

				if (hitEntity->HasComponent<Ndk::PhysicsComponent2D>())
				{
					Ndk::PhysicsComponent2D& hitEntityPhys = hitEntity->GetComponent<Ndk::PhysicsComponent2D>();
					hitEntityPhys.AddImpulse(Nz::Vector2f::Normalize(hitInfo.hitPos - startPos) * pushbackForce);
				}
			}*/
		};

		elementMetatable["Shoot"] = sol::overload(
			LuaFunction(shootFunc),
			LuaFunction([=](const sol::table& weaponTable, Nz::Vector2f startPos, Nz::Vector2f direction, Nz::UInt16 damage) { shootFunc(weaponTable, startPos, direction, damage); })
		);
	}
}
