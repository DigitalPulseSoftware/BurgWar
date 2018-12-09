// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/Scripting/ServerEntityStore.hpp>
#include <NDK/Components.hpp>
#include <NDK/LuaAPI.hpp>
#include <Nazara/Utility/Image.hpp>
#include <Shared/Gamemode.hpp>
#include <Shared/Components/HealthComponent.hpp>
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
	const Ndk::EntityHandle& ServerEntityStore::InstantiateEntity(Ndk::World& world, std::size_t entityIndex)
	{
		auto& entityClass = *GetElement(entityIndex);

		Nz::LuaState& state = GetLuaState();

		std::string spritePath;
		bool playerControlled;
		float scale;
		try
		{
			state.PushReference(entityClass.tableRef);
			Nz::CallOnExit popOnExit([&] { state.Pop(); });

			playerControlled = state.CheckField<bool>("PlayerControlled");
			scale = state.CheckField<float>("Scale");
			spritePath = state.CheckField<std::string>("Sprite");
		}
		catch (const std::exception& e)
		{
			std::cerr << "Failed to get entity class \"" << entityClass.name << "\" informations: " << e.what() << std::endl;
			return Ndk::EntityHandle::InvalidHandle;
		}


		Nz::ImageRef boxImage = Nz::ImageManager::Get(spritePath);
		Nz::Vector2f imageSize = Nz::Vector2f(Nz::Vector3f(boxImage->GetSize())) * scale;

		const Ndk::EntityHandle& entity = world.CreateEntity();
		entity->AddComponent<Ndk::NodeComponent>();

		if (entityClass.maxHealth > 0)
		{
			auto& healthComponent = entity->AddComponent<HealthComponent>(entityClass.maxHealth);
			healthComponent.OnHealthChange.Connect([](HealthComponent* health)
			{
				auto& entityScript = health->GetEntity()->GetComponent<ScriptComponent>();

				entityScript.ExecuteCallback("OnHealthChange");
			});

			healthComponent.OnDying.Connect([&](HealthComponent* health, const Ndk::EntityHandle& attacker)
			{
				const Ndk::EntityHandle& entity = health->GetEntity();
				auto& entityScript = entity->GetComponent<ScriptComponent>();

				entityScript.ExecuteCallback("OnDeath", [&](Nz::LuaState& state) {
					state.Push(attacker);
					return 1;
				});
			});

			healthComponent.OnDied.Connect([&](const HealthComponent* health, const Ndk::EntityHandle& attacker)
			{
				const Ndk::EntityHandle& entity = health->GetEntity();
				auto& entityScript = entity->GetComponent<ScriptComponent>();

				entityScript.ExecuteCallback("OnDied", [&](Nz::LuaState& state) {
					state.Push(attacker);
					return 1;
				});
			});
		}

		if (entityClass.isNetworked)
			entity->AddComponent<NetworkSyncComponent>(entityClass.fullName);

		if (playerControlled)
			entity->AddComponent<PlayerMovementComponent>();

		if (!InitializeEntity(entityClass, entity))
			entity->Kill();

		return entity;
	}

	void ServerEntityStore::InitializeElementTable(Nz::LuaState& state)
	{
		SharedEntityStore::InitializeElementTable(state);

		state.PushField("IsNetworked", false);
	}

	void ServerEntityStore::InitializeElement(Nz::LuaState& state, ScriptedEntity& element)
	{
		SharedEntityStore::InitializeElement(state, element);

		element.isNetworked = state.CheckField<bool>("IsNetworked");
		element.maxHealth = state.CheckField<Nz::UInt16>("MaxHealth", 0, -1);
	}
}
