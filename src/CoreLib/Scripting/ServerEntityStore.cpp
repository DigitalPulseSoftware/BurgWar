// Copyright (C) 2020 Jérôme Leclercq
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
#include <CoreLib/Scripting/ScriptingUtils.hpp>
#include <CoreLib/Systems/NetworkSyncSystem.hpp>
#include <CoreLib/Systems/PlayerMovementSystem.hpp>

namespace bw
{
	const Ndk::EntityHandle& ServerEntityStore::CreateEntity(TerrainLayer& layer, std::size_t entityIndex, EntityId uniqueId, const Nz::Vector2f& position, const Nz::DegreeAnglef& rotation, const PropertyValueMap& properties, const Ndk::EntityHandle& parent) const
	{
		const auto& entityClass = GetElement(entityIndex);

		bool hasInputs = entityClass->elementTable.get_or("HasInputs", false);
		bool playerControlled = entityClass->elementTable.get_or("PlayerControlled", false);

		const Ndk::EntityHandle& entity = SharedEntityStore::CreateEntity(layer.GetWorld(), entityClass, properties);
		entity->AddComponent<MatchComponent>(layer.GetMatch(), layer.GetLayerIndex(), uniqueId);

		auto& node = entity->AddComponent<Ndk::NodeComponent>();
		node.SetPosition(position);
		node.SetRotation(rotation);

		if (parent)
			node.SetParent(parent);

		if (entityClass->maxHealth > 0)
		{
			auto& healthComponent = entity->AddComponent<HealthComponent>(entityClass->maxHealth);
			healthComponent.OnDamage.Connect([](HealthComponent* health, Nz::UInt16& damageValue, const Ndk::EntityHandle& source)
			{
				auto& entityScript = health->GetEntity()->GetComponent<ScriptComponent>();

				if (auto ret = entityScript.ExecuteCallback<ElementEvent::TakeDamage>(damageValue, TranslateEntityToLua(source)); ret.has_value())
					damageValue = *ret;
			});

			healthComponent.OnHealthChange.Connect([](HealthComponent* health, Nz::UInt16 newHealth, const Ndk::EntityHandle& source)
			{
				auto& entityScript = health->GetEntity()->GetComponent<ScriptComponent>();

				entityScript.ExecuteCallback<ElementEvent::HealthUpdate>(newHealth, TranslateEntityToLua(source));
			});

			healthComponent.OnDying.Connect([&](HealthComponent* health, const Ndk::EntityHandle& attacker)
			{
				const Ndk::EntityHandle& entity = health->GetEntity();
				auto& entityScript = entity->GetComponent<ScriptComponent>();

				entityScript.ExecuteCallback<ElementEvent::Death>(TranslateEntityToLua(attacker));
			});

			healthComponent.OnDied.Connect([&](const HealthComponent* health, const Ndk::EntityHandle& attacker)
			{
				const Ndk::EntityHandle& entity = health->GetEntity();
				auto& entityScript = entity->GetComponent<ScriptComponent>();

				entityScript.ExecuteCallback<ElementEvent::Died>(TranslateEntityToLua(attacker));
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

		if (hasInputs)
			entity->AddComponent<InputComponent>();

		bwLog(GetLogger(), LogLevel::Debug, "Created entity {} on layer {} of type {}", uniqueId, layer.GetLayerIndex(), GetElement(entityIndex)->fullName);

		return entity;
	}

	bool ServerEntityStore::InitializeEntity(const Ndk::EntityHandle& entity) const
	{
		const auto& entityScript = entity->GetComponent<ScriptComponent>();
		return SharedEntityStore::InitializeEntity(static_cast<const ScriptedEntity&>(*entityScript.GetElement()), entity);
	}

	const Ndk::EntityHandle& ServerEntityStore::InstantiateEntity(TerrainLayer& layer, std::size_t entityIndex, EntityId uniqueId, const Nz::Vector2f& position, const Nz::DegreeAnglef& rotation, const PropertyValueMap& properties, const Ndk::EntityHandle& parent) const
	{
		const Ndk::EntityHandle& entity = CreateEntity(layer, entityIndex, uniqueId, position, rotation, properties, parent);
		if (!entity)
			return Ndk::EntityHandle::InvalidHandle;

		if (!InitializeEntity(entity))
		{
			entity->Kill();
			return Ndk::EntityHandle::InvalidHandle;
		}

		return entity;
	}

	void ServerEntityStore::InitializeElementTable(sol::main_table& elementTable)
	{
		SharedEntityStore::InitializeElementTable(elementTable);
	}

	void ServerEntityStore::InitializeElement(sol::main_table& elementTable, ScriptedEntity& element)
	{
		SharedEntityStore::InitializeElement(elementTable, element);

		element.isNetworked = elementTable.get_or("IsNetworked", false);
		element.maxHealth = elementTable.get_or("MaxHealth", Nz::UInt16(0));
	}
}
