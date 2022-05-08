// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/ServerEntityStore.hpp>
#include <CoreLib/PlayerInputController.hpp>
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
#include <Nazara/Utility/Image.hpp>
#include <Nazara/Utility/Components/NodeComponent.hpp>
#include <entt/entt.hpp>

namespace bw
{
	entt::entity ServerEntityStore::CreateEntity(TerrainLayer& layer, std::size_t entityIndex, EntityId uniqueId, const Nz::Vector2f& position, const Nz::DegreeAnglef& rotation, const PropertyValueMap& properties, entt::entity parent) const
	{
		const auto& entityClass = GetElement(entityIndex);

		bool hasInputs = entityClass->elementTable.get_or("HasInputs", false);
		bool playerControlled = entityClass->elementTable.get_or("PlayerControlled", false);

		entt::registry& registry = layer.GetWorld();

		entt::entity entity = SharedEntityStore::CreateEntity(layer.GetWorld(), entityClass, properties);
		registry.emplace<MatchComponent>(entity, layer.GetMatch(), layer.GetLayerIndex(), uniqueId);

		auto& node = registry.emplace<Nz::NodeComponent>(entity);
		node.SetPosition(position);
		node.SetRotation(rotation);

		if (parent != entt::null)
			node.SetParent(registry, parent);

		if (entityClass->maxHealth > 0)
		{
			auto& healthComponent = registry.emplace<HealthComponent>(entity, registry, entity, entityClass->maxHealth);
			healthComponent.OnDamage.Connect([](HealthComponent* health, Nz::UInt16& damageValue, entt::entity source)
			{
				entt::registry& registry = health->GetRegistry();
				auto& entityScript = registry.get<ScriptComponent>(health->GetEntity());

				if (auto ret = entityScript.ExecuteCallback<ElementEvent::TakeDamage>(damageValue, TranslateEntityToLua(registry, source)); ret.has_value())
					damageValue = *ret;
			});

			healthComponent.OnHealthChange.Connect([](HealthComponent* health, Nz::UInt16 newHealth, entt::entity source)
			{
				entt::registry& registry = health->GetRegistry();
				auto& entityScript = registry.get<ScriptComponent>(health->GetEntity());

				entityScript.ExecuteCallback<ElementEvent::HealthUpdate>(newHealth, TranslateEntityToLua(registry, source));
			});

			healthComponent.OnDying.Connect([&](HealthComponent* health, entt::entity attacker)
			{
				entt::registry& registry = health->GetRegistry();
				auto& entityScript = registry.get<ScriptComponent>(health->GetEntity());

				entityScript.ExecuteCallback<ElementEvent::Death>(TranslateEntityToLua(registry, attacker));
			});

			healthComponent.OnDie.Connect([&](const HealthComponent* health, entt::entity attacker)
			{
				entt::registry& registry = health->GetRegistry();
				auto& entityScript = registry.get<ScriptComponent>(health->GetEntity());

				entityScript.ExecuteCallback<ElementEvent::Died>(TranslateEntityToLua(registry, attacker));
			});
		}

		if (entityClass->isNetworked)
		{
			// Not quite sure about this, maybe parent handling should be automatic?

			if (parent != entt::null && registry.try_get<NetworkSyncComponent>(parent))
				registry.emplace<NetworkSyncComponent>(entity, entityClass->fullName, parent);
			else
				registry.emplace<NetworkSyncComponent>(entity, entityClass->fullName);
		}

		if (playerControlled)
			registry.emplace<PlayerMovementComponent>(entity);

		if (hasInputs)
			registry.emplace<InputComponent>(entity, std::make_shared<PlayerInputController>());

		bwLog(GetLogger(), LogLevel::Debug, "Created entity {} on layer {} of type {}", uniqueId, layer.GetLayerIndex(), GetElement(entityIndex)->fullName);

		return entity;
	}

	bool ServerEntityStore::InitializeEntity(entt::registry& registry, entt::entity entity) const
	{
		const auto& entityScript = registry.get<ScriptComponent>(entity);
		return SharedEntityStore::InitializeEntity(static_cast<const ScriptedEntity&>(*entityScript.GetElement()), entity);
	}

	entt::entity ServerEntityStore::InstantiateEntity(TerrainLayer& layer, std::size_t entityIndex, EntityId uniqueId, const Nz::Vector2f& position, const Nz::DegreeAnglef& rotation, const PropertyValueMap& properties, entt::entity parent) const
	{
		entt::registry& registry = layer.GetWorld();

		entt::entity entity = CreateEntity(layer, entityIndex, uniqueId, position, rotation, properties, parent);
		if (entity != entt::null)
			return entt::null;

		if (!InitializeEntity(registry, entity))
		{
			registry.destroy(entity);
			return entt::null;
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
