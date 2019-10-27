// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/ServerEntityStore.hpp>
#include <NDK/Components.hpp>
#include <Nazara/Utility/Image.hpp>
#include <CoreLib/TerrainLayer.hpp>
#include <CoreLib/Components/HealthComponent.hpp>
#include <CoreLib/Components/MatchComponent.hpp>
#include <CoreLib/Components/NetworkSyncComponent.hpp>
#include <CoreLib/Components/PlayerControlledComponent.hpp>
#include <CoreLib/Components/PlayerMovementComponent.hpp>
#include <CoreLib/Components/ScriptComponent.hpp>
#include <CoreLib/Systems/NetworkSyncSystem.hpp>
#include <CoreLib/Systems/PlayerMovementSystem.hpp>

namespace bw
{
	const Ndk::EntityHandle& ServerEntityStore::InstantiateEntity(TerrainLayer& layer, std::size_t entityIndex, const Nz::Vector2f& position, const Nz::DegreeAnglef& rotation, const EntityProperties& properties) const
	{
		const auto& entityClass = GetElement(entityIndex);

		bool playerControlled;
		try
		{
			playerControlled = entityClass->elementTable["PlayerControlled"];
		}
		catch (const std::exception& e)
		{
			bwLog(GetLogger(), LogLevel::Error, "Failed to get entity class \"{0}\" informations: {1}", entityClass->name, e.what());
			return Ndk::EntityHandle::InvalidHandle;
		}


		const Ndk::EntityHandle& entity = CreateEntity(layer.GetWorld(), entityClass, properties);
		entity->AddComponent<MatchComponent>(layer.GetMatch(), layer.GetLayerIndex());

		auto& node = entity->AddComponent<Ndk::NodeComponent>();
		node.SetPosition(position);
		node.SetRotation(rotation);

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
