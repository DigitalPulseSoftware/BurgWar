// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/SharedWeaponLibrary.hpp>
#include <CoreLib/Scripting/AbstractScriptingLibrary.hpp>
#include <CoreLib/Components/HealthComponent.hpp>
#include <NDK/World.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <NDK/Components/PhysicsComponent2D.hpp>
#include <NDK/Systems/PhysicsSystem2D.hpp>
#include <sol3/sol.hpp>

namespace bw
{
	void SharedWeaponLibrary::RegisterLibrary(sol::table& elementMetatable)
	{
		RegisterSharedLibrary(elementMetatable);
	}

	void SharedWeaponLibrary::RegisterSharedLibrary(sol::table& elementMetatable)
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

		elementMetatable["GetDirection"] = [](const sol::table& entityTable)
		{
			const Ndk::EntityHandle& entity = AssertScriptEntity(entityTable);

			auto& nodeComponent = entity->GetComponent<Ndk::NodeComponent>();

			Nz::Vector2f direction(nodeComponent.GetRotation() * Nz::Vector2f::UnitX());
			if (nodeComponent.GetScale().x < 0.f)
				direction = -direction;

			return direction;
		};

		elementMetatable["GetPosition"] = [](const sol::table& entityTable)
		{
			const Ndk::EntityHandle& entity = AssertScriptEntity(entityTable);

			auto& nodeComponent = entity->GetComponent<Ndk::NodeComponent>();
			return Nz::Vector2f(nodeComponent.GetPosition());
		};

		elementMetatable["GetRotation"] = [](const sol::table& entityTable)
		{
			const Ndk::EntityHandle& entity = AssertScriptEntity(entityTable);

			auto& nodeComponent = entity->GetComponent<Ndk::NodeComponent>();
			return nodeComponent.GetRotation().ToEulerAngles().roll; //<FIXME: not very efficient
		};

		//FIXME: Move this is SharedElementLibrary?
		elementMetatable["IsLookingRight"] = [](const sol::table& entityTable)
		{
			const Ndk::EntityHandle& entity = AssertScriptEntity(entityTable);

			auto& nodeComponent = entity->GetComponent<Ndk::NodeComponent>();
			return nodeComponent.GetScale().x > 0.f;
		};
	}
}
