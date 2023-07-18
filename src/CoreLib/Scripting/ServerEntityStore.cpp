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
	entt::handle ServerEntityStore::CreateEntity(TerrainLayer& layer, std::size_t entityIndex, EntityId uniqueId, const Nz::Vector2f& position, const Nz::DegreeAnglef& rotation, const PropertyValueMap& properties, entt::handle parent) const
	{
		const auto& entityClass = GetElement(entityIndex);

		bool hasInputs = entityClass->elementTable.get_or("HasInputs", false);
		bool playerControlled = entityClass->elementTable.get_or("PlayerControlled", false);

		entt::registry& registry = layer.GetWorld();

		entt::handle entity = SharedEntityStore::CreateEntity(layer.GetWorld(), entityClass, properties);
		entity.emplace<MatchComponent>(layer.GetMatch(), layer.GetLayerIndex(), uniqueId);

		auto& node = registry.emplace<Nz::NodeComponent>(entity);
		node.SetPosition(position);
		node.SetRotation(rotation);

		if (parent)
			node.SetParent(parent);

		if (entityClass->maxHealth > 0)
		{
			auto& healthComponent = entity.emplace<HealthComponent>(entity, entityClass->maxHealth);
			healthComponent.OnDamage.Connect([](HealthComponent* health, Nz::UInt16& damageValue, entt::handle source)
			{
				auto& entityScript = health->GetHandle().get<ScriptComponent>();

				if (auto ret = entityScript.ExecuteCallback<ElementEvent::TakeDamage>(damageValue, TranslateEntityToLua(source)); ret.has_value())
					damageValue = *ret;
			});

			healthComponent.OnHealthChange.Connect([](HealthComponent* health, Nz::UInt16 newHealth, entt::handle source)
			{
				auto& entityScript = health->GetHandle().get<ScriptComponent>();

				entityScript.ExecuteCallback<ElementEvent::HealthUpdate>(newHealth, TranslateEntityToLua(source));
			});

			healthComponent.OnDying.Connect([&](HealthComponent* health, entt::handle attacker)
			{
				auto& entityScript = health->GetHandle().get<ScriptComponent>();

				entityScript.ExecuteCallback<ElementEvent::Death>(TranslateEntityToLua(attacker));
			});

			healthComponent.OnDie.Connect([&](const HealthComponent* health, entt::handle attacker)
			{
				auto& entityScript = health->GetHandle().get<ScriptComponent>();

				entityScript.ExecuteCallback<ElementEvent::Died>(TranslateEntityToLua(attacker));
			});
		}

		if (entityClass->isNetworked)
		{
			// Not quite sure about this, maybe parent handling should be automatic?
			NetworkSyncSystem& networkSyncSystem = layer.GetNetworkSyncSystem();

			if (parent && parent.try_get<NetworkSyncComponent>())
				entity.emplace<NetworkSyncComponent>(networkSyncSystem, entityClass->fullName, parent);
			else
				entity.emplace<NetworkSyncComponent>(networkSyncSystem, entityClass->fullName);
		}

		if (playerControlled)
			entity.emplace<PlayerMovementComponent>();

		if (hasInputs)
			entity.emplace<InputComponent>(entity, std::make_shared<PlayerInputController>());

		bwLog(GetLogger(), LogLevel::Debug, "Created entity {} on layer {} of type {}", uniqueId, layer.GetLayerIndex(), GetElement(entityIndex)->fullName);

		return entity;
	}

	bool ServerEntityStore::InitializeEntity(entt::handle entity) const
	{
		const auto& entityScript = entity.get<ScriptComponent>();
		return SharedEntityStore::InitializeEntity(static_cast<const ScriptedEntity&>(*entityScript.GetElement()), entity);
	}

	entt::handle ServerEntityStore::InstantiateEntity(TerrainLayer& layer, std::size_t entityIndex, EntityId uniqueId, const Nz::Vector2f& position, const Nz::DegreeAnglef& rotation, const PropertyValueMap& properties, entt::handle parent) const
	{
		entt::registry& registry = layer.GetWorld();

		entt::handle entity = CreateEntity(layer, entityIndex, uniqueId, position, rotation, properties, parent);
		if (!entity)
			return entt::handle{};

		if (!InitializeEntity(entity))
		{
			registry.destroy(entity);
			return entt::handle{};
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
