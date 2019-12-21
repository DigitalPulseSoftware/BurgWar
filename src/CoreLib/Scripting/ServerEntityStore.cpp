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
	const Ndk::EntityHandle& ServerEntityStore::InstantiateEntity(TerrainLayer& layer, std::size_t entityIndex, Nz::Int64 uniqueId, const Nz::Vector2f& position, const Nz::DegreeAnglef& rotation, const EntityProperties& properties, const Ndk::EntityHandle& parent) const
	{
		const auto& entityClass = GetElement(entityIndex);

		bool playerControlled = entityClass->elementTable.get_or("PlayerControlled", false);

		const Ndk::EntityHandle& entity = CreateEntity(layer.GetWorld(), entityClass, properties);
		entity->AddComponent<MatchComponent>(layer.GetMatch(), layer.GetLayerIndex(), uniqueId);

		auto& node = entity->AddComponent<Ndk::NodeComponent>();
		node.SetPosition(position);
		node.SetRotation(rotation);

		if (parent)
			node.SetParent(parent);

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
		{
			// Not quite sure about this, maybe parent handling should be automatic?
			if (parent && parent->HasComponent<NetworkSyncComponent>())
				entity->AddComponent<NetworkSyncComponent>(entityClass->fullName, parent);
			else
				entity->AddComponent<NetworkSyncComponent>(entityClass->fullName);
		}

		if (playerControlled)
			entity->AddComponent<PlayerMovementComponent>();

		if (!InitializeEntity(*entityClass, entity))
			entity->Kill();

		bwLog(GetLogger(), LogLevel::Debug, "Created entity {} on layer {} of type {}", uniqueId, layer.GetLayerIndex(), GetElement(entityIndex)->fullName);

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
		element.maxHealth = elementTable.get_or("MaxHealth", Nz::UInt16(0));
	}
}
