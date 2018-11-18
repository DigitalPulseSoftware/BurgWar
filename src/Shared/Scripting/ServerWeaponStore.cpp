// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/Scripting/ServerWeaponStore.hpp>
#include <Nazara/Utility/Image.hpp>
#include <NDK/Components.hpp>
#include <NDK/LuaAPI.hpp>
#include <NDK/Systems.hpp>
#include <NDK/World.hpp>
#include <Shared/Components/NetworkSyncComponent.hpp>
#include <Shared/Components/PlayerControlledComponent.hpp>
#include <Shared/Components/PlayerMovementComponent.hpp>
#include <Shared/Components/ScriptComponent.hpp>
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
		auto& weaponNode = weapon->AddComponent<Ndk::NodeComponent>();
		weapon->AddComponent<NetworkSyncComponent>(weaponClass.fullName, parent);

		weaponNode.SetParent(parent);
		weaponNode.SetPosition(weaponClass.weaponOffset);

		state.PushTable();

		state.PushField("Entity", weapon);

		state.PushReference(weaponClass.tableRef);
		state.SetMetatable(-2);

		int tableRef = state.CreateReference();

		weapon->AddComponent<ScriptComponent>(GetScriptingContext(), tableRef);

		return weapon;
	}

	void ServerWeaponStore::InitializeElementTable(Nz::LuaState& state)
	{
		state.PushFunction([](Nz::LuaState& state) -> int
		{
			Ndk::EntityHandle entity = state.CheckField<Ndk::EntityHandle>("Entity", 1);

			Nz::UInt16 damage = state.CheckInteger(2);
			float range = state.CheckNumber(3);

			Ndk::World* world = entity->GetWorld();
			assert(world);

			auto& nodeComponent = entity->GetComponent<Ndk::NodeComponent>();

			Nz::Vector2f pos = Nz::Vector2f(nodeComponent.GetPosition());

			std::vector<Ndk::EntityHandle> hitEntities;
			world->GetSystem<Ndk::PhysicsSystem2D>().RegionQuery(Nz::Rectf(pos.x - range / 2.f, pos.y - range / 2.f, range, range), 0, 0xFFFFFFFF, 0xFFFFFFFF, &hitEntities);

			for (const Ndk::EntityHandle& hitEntity : hitEntities)
			{
				if (hitEntity->HasComponent<HealthComponent>())
					hitEntity->GetComponent<HealthComponent>().Damage(damage, entity);

				std::cout << hitEntity << std::endl;
			}

			return 0;
		});
		state.SetField("DealDamage");
	}

	void ServerWeaponStore::InitializeElement(Nz::LuaState& state, ScriptedWeapon& weapon)
	{
		weapon.weaponOffset = state.CheckField<Nz::Vector2f>("WeaponOffset", Nz::Vector2f::Zero());

		// Actually server doesn't care about those
		weapon.scale = state.CheckField<float>("Scale");
		weapon.spriteName = state.CheckField<std::string>("Sprite");
	}
}
