// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <GameLibShared/Scripting/ServerEntityStore.hpp>
#include <NDK/Components.hpp>
#include <Nazara/Utility/Image.hpp>
#include <GameLibShared/Components/HealthComponent.hpp>
#include <GameLibShared/Components/HealthComponent.hpp>
#include <GameLibShared/Components/NetworkSyncComponent.hpp>
#include <GameLibShared/Components/PlayerControlledComponent.hpp>
#include <GameLibShared/Components/PlayerMovementComponent.hpp>
#include <GameLibShared/Components/ScriptComponent.hpp>
#include <GameLibShared/Systems/NetworkSyncSystem.hpp>
#include <GameLibShared/Systems/PlayerMovementSystem.hpp>
#include <iostream>

namespace bw
{
	const Ndk::EntityHandle& ServerEntityStore::InstantiateEntity(Ndk::World& world, std::size_t entityIndex, const EntityProperties& properties)
	{
		const auto& entityClass = GetElement(entityIndex);

		std::string spritePath;
		bool playerControlled;
		float scale;
		try
		{
			playerControlled = entityClass->elementTable["PlayerControlled"];
			scale = entityClass->elementTable["Scale"];
			spritePath = entityClass->elementTable["Sprite"];
		}
		catch (const std::exception& e)
		{
			std::cerr << "Failed to get entity class \"" << entityClass->name << "\" informations: " << e.what() << std::endl;
			return Ndk::EntityHandle::InvalidHandle;
		}


		Nz::ImageRef boxImage = Nz::ImageManager::Get(spritePath);
		Nz::Vector2f imageSize = Nz::Vector2f(Nz::Vector3f(boxImage->GetSize())) * scale;

		const Ndk::EntityHandle& entity = CreateEntity(world, entityClass, properties);

		entity->AddComponent<Ndk::NodeComponent>();

		if (entityClass->maxHealth > 0)
		{
			auto& healthComponent = entity->AddComponent<HealthComponent>(entityClass->maxHealth);
			healthComponent.OnHealthChange.Connect([](HealthComponent* health)
			{
				auto& entityScript = health->GetEntity()->GetComponent<ScriptComponent>();

				entityScript.ExecuteCallback("OnHealthChange");
			});

			healthComponent.OnDying.Connect([&](HealthComponent* health, const Ndk::EntityHandle& attacker)
			{
				const Ndk::EntityHandle& entity = health->GetEntity();
				auto& entityScript = entity->GetComponent<ScriptComponent>();

				entityScript.ExecuteCallback("OnDeath", attacker);
			});

			healthComponent.OnDied.Connect([&](const HealthComponent* health, const Ndk::EntityHandle& attacker)
			{
				const Ndk::EntityHandle& entity = health->GetEntity();
				auto& entityScript = entity->GetComponent<ScriptComponent>();

				entityScript.ExecuteCallback("OnDied", attacker);
			});
		}

		if (entityClass->isNetworked)
			entity->AddComponent<NetworkSyncComponent>(entityClass->fullName);

		if (playerControlled)
			entity->AddComponent<PlayerMovementComponent>();

		if (!InitializeEntity(*entityClass, entity))
			entity->Kill();

		return entity;
	}

	void ServerEntityStore::InitializeElementTable(sol::table& elementTable)
	{
		SharedEntityStore::InitializeElementTable(elementTable);

		elementTable["IsNetworked"] = false;
	}

	void ServerEntityStore::InitializeElement(sol::table& elementTable, ScriptedEntity& element)
	{
		SharedEntityStore::InitializeElement(elementTable, element);

		element.isNetworked = elementTable["IsNetworked"];
		element.maxHealth = elementTable.get_or("MaxHealth", 0);
	}
}
