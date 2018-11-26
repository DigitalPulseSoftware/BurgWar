// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/Scripting/ServerWeaponStore.hpp>
#include <Shared/BurgApp.hpp>
#include <Shared/Components/AnimationComponent.hpp>
#include <Shared/Components/NetworkSyncComponent.hpp>
#include <Shared/Components/PlayerControlledComponent.hpp>
#include <Shared/Components/PlayerMovementComponent.hpp>
#include <Shared/Components/ScriptComponent.hpp>
#include <Shared/Systems/NetworkSyncSystem.hpp>
#include <Shared/Systems/PlayerControlledSystem.hpp>
#include <Shared/Systems/PlayerMovementSystem.hpp>
#include <Nazara/Math/Rect.hpp>
#include <Nazara/Utility/Image.hpp>
#include <NDK/Components.hpp>
#include <NDK/LuaAPI.hpp>
#include <NDK/Systems.hpp>
#include <NDK/World.hpp>
#include <iostream>

namespace bw
{
	const Ndk::EntityHandle& ServerWeaponStore::InstantiateWeapon(Ndk::World& world, std::size_t weaponIndex, const Ndk::EntityHandle& parent)
	{
		auto& weaponClass = *GetElement(weaponIndex);

		Nz::LuaState& state = GetLuaState();

		const Ndk::EntityHandle& weapon = world.CreateEntity();
		weapon->AddComponent<NetworkSyncComponent>(weaponClass.fullName, parent);

		SharedWeaponStore::InitializeWeapon(weaponClass, weapon, parent);

		return weapon;
	}

	void ServerWeaponStore::InitializeElementTable(Nz::LuaState& state)
	{
		SharedWeaponStore::InitializeElementTable(state);

		state.PushFunction([](Nz::LuaState& state) -> int
		{
			Ndk::EntityHandle entity = state.CheckField<Ndk::EntityHandle>("Entity", 1);

			int index = 2;
			Nz::UInt16 damage = state.Check<Nz::UInt16>(&index);
			Nz::Rectf damageZone = state.Check<Nz::Rectf>(&index);

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

			return 0;
		});
		state.SetField("DealDamage");

		state.PushFunction([this](Nz::LuaState& state) -> int
		{
			Ndk::EntityHandle entity = state.CheckField<Ndk::EntityHandle>("Entity", 1);

			int index = 2;
			std::string animationName = state.Check<std::string>(&index);

			if (!entity->HasComponent<AnimationComponent>())
				state.Error("Entity has no animations");

			auto& entityAnimation = entity->GetComponent<AnimationComponent>();
			const auto& animationStore = entityAnimation.GetAnimationStore();

			if (std::size_t animId = animationStore->FindAnimationByName(animationName); animId != animationStore->InvalidId)
				entityAnimation.Play(animId, m_application.GetAppTime());
			else
				state.Error("Entity has no animation \"" + animationName + "\"");

			return 0;
		});
		state.SetField("PlayAnim");

		state.PushFunction([](Nz::LuaState& state) -> int
		{
			Ndk::EntityHandle entity = state.CheckField<Ndk::EntityHandle>("Entity", 1);
			if (!entity->HasComponent<AnimationComponent>())
			{
				state.PushBoolean(false);
				return 1;
			}

			auto& entityAnimation = entity->GetComponent<AnimationComponent>();
			state.PushBoolean(entityAnimation.IsPlaying());
			return 1;
		});
		state.SetField("IsPlayingAnimation");
	}

	void ServerWeaponStore::InitializeElement(Nz::LuaState& state, ScriptedWeapon& weapon)
	{
		SharedWeaponStore::InitializeElement(state, weapon);
	}
}
