// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/MatchClientVisibility.hpp>
#include <Nazara/Core/StackArray.hpp>
#include <Nazara/Core/StackVector.hpp>
#include <CoreLib/Protocol/Packets.hpp>
#include <CoreLib/MatchClientSession.hpp>
#include <CoreLib/Terrain.hpp>
#include <cassert>
#include <queue>

namespace bw
{
	void MatchClientVisibility::ShowLayer(LayerIndex layerIndex)
	{
		if (auto it = m_layers.find(layerIndex); it != m_layers.end())
		{
			Layer& layer = *(it.value());
			layer.visibilityCounter++;
		}
		else
		{
			auto& layer = *m_layers.emplace(layerIndex, std::make_unique<Layer>()).first.value();

			Terrain& terrain = m_match.GetTerrain();
			assert(layerIndex < terrain.GetLayerCount());

			/* Create all newly visible entities */
			TerrainLayer& terrainLayer = terrain.GetLayer(layerIndex);
			NetworkSyncSystem& syncSystem = terrainLayer.GetWorld().GetSystem<NetworkSyncSystem>();
			
			layer.onEntityCreatedSlot.Connect(syncSystem.OnEntityCreated, [this](NetworkSyncSystem* syncSystem, const NetworkSyncSystem::EntityCreation& entityCreation)
			{
				HandleEntityCreation(syncSystem->GetLayer().GetLayerIndex(), entityCreation);
			});

			layer.onEntityDeletedSlot.Connect(syncSystem.OnEntityDeleted, [this](NetworkSyncSystem* syncSystem, const NetworkSyncSystem::EntityDestruction& entityDestruction)
			{
				HandleEntityRemove(syncSystem->GetLayer().GetLayerIndex(), entityDestruction.entityId, false);
			});

			layer.onEntityInvalidated.Connect(syncSystem.OnEntityInvalidated, [this, layerIndex](NetworkSyncSystem*, const NetworkSyncSystem::EntityMovement& entityMovement)
			{
				assert(m_layers.find(layerIndex) != m_layers.end());
				Layer& layer = *m_layers[layerIndex];

				if (!layer.visibleEntities.UnboundedTest(entityMovement.entityId))
					return;

				layer.staticMovementUpdateEvents[entityMovement.entityId] = entityMovement;
			});

			layer.onEntityPlayAnimation.Connect(syncSystem.OnEntityPlayAnimation, [this, layerIndex](NetworkSyncSystem*, const NetworkSyncSystem::EntityPlayAnimation& entityPlayAnimation)
			{
				assert(m_layers.find(layerIndex) != m_layers.end());
				Layer& layer = *m_layers[layerIndex];

				if (!layer.visibleEntities.UnboundedTest(entityPlayAnimation.entityId))
					return;

				layer.playAnimationEvents[entityPlayAnimation.entityId] = entityPlayAnimation;
				m_pendingEvents.Set(VisibilityEventType::PlayAnimation);
			});

			layer.onEntityDeath.Connect(syncSystem.OnEntityDeath, [this](NetworkSyncSystem* syncSystem, const NetworkSyncSystem::EntityDeath& entityDeath)
			{
				HandleEntityRemove(syncSystem->GetLayer().GetLayerIndex(), entityDeath.entityId, true);
			});

			layer.onEntitiesHealthUpdate.Connect(syncSystem.OnEntitiesHealthUpdate, [this, layerIndex](NetworkSyncSystem*, const NetworkSyncSystem::EntityHealth* events, std::size_t entityCount)
			{
				assert(m_layers.find(layerIndex) != m_layers.end());
				Layer& layer = *m_layers[layerIndex];
				
				for (std::size_t i = 0; i < entityCount; ++i)
				{
					if (!layer.visibleEntities.UnboundedTest(events[i].entityId))
						continue;

					layer.healthUpdateEvents[events[i].entityId] = events[i];
					m_pendingEvents.Set(VisibilityEventType::HealthUpdate);
				}
			});

			layer.onEntitiesInputUpdate.Connect(syncSystem.OnEntitiesInputUpdate, [this, layerIndex](NetworkSyncSystem*, const NetworkSyncSystem::EntityInputs* events, std::size_t entityCount)
			{
				assert(m_layers.find(layerIndex) != m_layers.end());
				Layer& layer = *m_layers[layerIndex];

				for (std::size_t i = 0; i < entityCount; ++i)
				{
					Nz::UInt64 entityKey = Nz::UInt64(layerIndex) << 32 | events[i].entityId;
					if (m_controlledEntities.find(entityKey) != m_controlledEntities.end())
						continue;

					if (!layer.visibleEntities.UnboundedTest(events[i].entityId))
						continue;

					layer.inputUpdateEvents[events[i].entityId] = events[i];
					m_pendingEvents.Set(VisibilityEventType::InputUpdate);
				}
			});

			m_newlyHiddenLayers.UnboundedReset(layerIndex);
			m_newlyVisibleLayers.UnboundedSet(layerIndex);
		}
	}

	void MatchClientVisibility::Update()
	{
		Nz::UInt16 networkTick = m_match.GetNetworkTick();

		// Handle hidden and shown layers
		if (m_newlyHiddenLayers.GetSize() != 0)
		{
			for (std::size_t i = m_newlyHiddenLayers.FindFirst(); i != m_newlyHiddenLayers.npos; i = m_newlyHiddenLayers.FindNext(i))
			{
				if (!m_clientVisibleLayers.UnboundedTest(i))
					continue;

				LayerIndex layerIndex = LayerIndex(i);

				// The client will destroy automatically all entities that belong to this layer
				Packets::DisableLayer disableLayer;
				disableLayer.layerIndex = layerIndex;
				disableLayer.stateTick = networkTick;
				
				m_session.SendPacket(disableLayer);

				m_clientVisibleLayers.UnboundedReset(layerIndex);
			}
			m_newlyHiddenLayers.Clear();
		}

		if (!m_pendingLayerUpdates.empty())
		{
			for (auto&& layerUpdate : m_pendingLayerUpdates)
			{
				Packets::PlayerLayer layerPacket;
				layerPacket.stateTick = networkTick;
				layerPacket.layerIndex = layerUpdate.layerIndex;
				layerPacket.localIndex = layerUpdate.localPlayerIndex;

				m_session.SendPacket(layerPacket);
			}

			m_pendingLayerUpdates.clear();
		}

		if (m_newlyVisibleLayers.GetSize() != 0)
		{
			PendingCreationEventMap pendingCreationMap; //< Used to resolve parenting

			for (std::size_t i = m_newlyVisibleLayers.FindFirst(); i != m_newlyVisibleLayers.npos; i = m_newlyVisibleLayers.FindNext(i))
			{
				LayerIndex layerIndex = LayerIndex(i);

				Terrain& terrain = m_match.GetTerrain();
				assert(layerIndex < terrain.GetLayerCount());

				/* Create all newly visible entities */
				TerrainLayer& terrainLayer = terrain.GetLayer(layerIndex);
				NetworkSyncSystem& syncSystem = terrainLayer.GetWorld().GetSystem<NetworkSyncSystem>();

				auto layerIt = m_layers.find(layerIndex);
				assert(layerIt != m_layers.end());
				Layer& layer = *layerIt.value();

				if (m_clientVisibleLayers.UnboundedTest(i))
				{
					for (const Ndk::EntityHandle& entity : syncSystem.GetEntities())
						layer.visibleEntities.UnboundedSet(entity->GetId());

					continue;
				}

				syncSystem.CreateEntities([&](const NetworkSyncSystem::EntityCreation* entitiesCreation, std::size_t entityCount)
				{
					for (std::size_t i = 0; i < entityCount; ++i)
					{
						if (!layer.visibleEntities.UnboundedTest(entitiesCreation[i].entityId))
							pendingCreationMap[entitiesCreation[i].entityId] = entitiesCreation[i];
					}
				});

				Packets::EnableLayer enableLayerPacket;
				enableLayerPacket.layerIndex = layerIndex;
				enableLayerPacket.stateTick = networkTick;

				std::function<void(PendingCreationEventMap::iterator it)> PushEntity;
				PushEntity = [&](PendingCreationEventMap::iterator it)
				{
					auto& entityId = it.key();
					auto& eventData = it.value();
					if (!eventData.has_value())
						return;

					auto HandleDependentEntity = [&](Nz::UInt32 entityId)
					{
						auto dependentIt = pendingCreationMap.find(entityId);
						if (dependentIt != pendingCreationMap.end() && dependentIt != it)
							PushEntity(dependentIt);
					};

					if (eventData->parent)
						HandleDependentEntity(static_cast<Nz::UInt32>(eventData->parent.value()));

					for (auto&& [layerIndex, entityIndex] : eventData->dependentIds)
						HandleDependentEntity(static_cast<Nz::UInt32>(entityIndex));

					auto& entityData = enableLayerPacket.layerEntities.emplace_back();
					entityData.id = eventData->entityId;
					FillEntityData(eventData.value(), entityData.data);

					layer.visibleEntities.UnboundedSet(entityId);

					eventData.reset();
				};

				for (auto it = pendingCreationMap.begin(); it != pendingCreationMap.end(); ++it)
					PushEntity(it);

				m_session.SendPacket(enableLayerPacket);

				m_clientVisibleLayers.UnboundedSet(layerIndex);

				pendingCreationMap.clear();
			}
			m_newlyVisibleLayers.Clear();
		}

		// Send packet in fixed order
		if (m_pendingEvents.Test(VisibilityEventType::Death))
		{
			m_entitiesDeathPacket.stateTick = networkTick;

			m_entitiesDeathPacket.entities.clear();
			m_entitiesDeathPacket.layers.clear();

			for (auto it = m_layers.begin(); it != m_layers.end(); ++it)
			{
				auto& layer = *it.value();
				if (layer.deathEvents.empty())
					continue;

				auto& layerData = m_entitiesDeathPacket.layers.emplace_back();
				layerData.layerIndex = it.key();
				layerData.entityCount = static_cast<Nz::UInt32>(layer.deathEvents.size());

				for (Nz::UInt32 entityId : layer.deathEvents)
				{
					auto& entityData = m_entitiesDeathPacket.entities.emplace_back();
					entityData.id = entityId;
				}
				layer.deathEvents.clear();
			}

			m_session.SendPacket(m_entitiesDeathPacket);

			m_pendingEvents.Clear(VisibilityEventType::Death);
		}

		if (m_pendingEvents.Test(VisibilityEventType::Destruction))
		{
			m_deleteEntitiesPacket.stateTick = networkTick;

			m_deleteEntitiesPacket.entities.clear();
			m_deleteEntitiesPacket.layers.clear();

			for (auto it = m_layers.begin(); it != m_layers.end(); ++it)
			{
				auto& layer = *it.value();
				if (layer.destructionEvents.empty())
					continue;

				LayerIndex layerIndex = it.key();

				auto& layerData = m_deleteEntitiesPacket.layers.emplace_back();
				layerData.layerIndex = layerIndex;
				layerData.entityCount = static_cast<Nz::UInt32>(layer.destructionEvents.size());

				for (Nz::UInt32 entityId : layer.destructionEvents)
				{
					auto& entityData = m_deleteEntitiesPacket.entities.emplace_back();
					entityData.id = entityId;
				}
				layer.destructionEvents.clear();
			}

			m_session.SendPacket(m_deleteEntitiesPacket);

			m_pendingEvents.Clear(VisibilityEventType::Destruction);
		}

		if (m_pendingEvents.Test(VisibilityEventType::Creation))
		{
			m_createEntitiesPacket.stateTick = networkTick;

			m_createEntitiesPacket.entities.clear();
			m_createEntitiesPacket.layers.clear();

			for (auto it = m_layers.begin(); it != m_layers.end(); ++it)
			{
				auto& layer = *it.value();
				if (layer.creationEvents.empty())
					continue;

				LayerIndex layerIndex = it.key();

				std::function<void(PendingCreationEventMap::iterator it)> PushEntity;
				PushEntity = [&](PendingCreationEventMap::iterator it)
				{
					auto& eventData = it.value();
					if (!eventData.has_value())
						return;

					auto HandleDependentEntity = [&](Nz::UInt32 entityId)
					{
						auto dependentIt = layer.creationEvents.find(entityId);
						if (dependentIt != layer.creationEvents.end() && dependentIt != it)
							PushEntity(dependentIt);
					};

					if (eventData->parent)
						HandleDependentEntity(static_cast<Nz::UInt32>(eventData->parent.value()));

					for (auto&& [layerIndex, entityIndex] : eventData->dependentIds)
						HandleDependentEntity(static_cast<Nz::UInt32>(entityIndex));

					auto& entityData = m_createEntitiesPacket.entities.emplace_back();
					entityData.id = eventData->entityId;
					FillEntityData(eventData.value(), entityData.data);

					eventData.reset();
				};

				auto& layerData = m_createEntitiesPacket.layers.emplace_back();
				layerData.layerIndex = layerIndex;
				layerData.entityCount = static_cast<Nz::UInt32>(layer.creationEvents.size());

				for (auto eventIt = layer.creationEvents.begin(); eventIt != layer.creationEvents.end(); ++eventIt)
					PushEntity(eventIt);
				layer.creationEvents.clear();
			}

			m_session.SendPacket(m_createEntitiesPacket);

			m_pendingEvents.Clear(VisibilityEventType::Creation);
		}

		if (m_pendingEvents.Test(VisibilityEventType::HealthUpdate))
		{
			m_healthUpdatePacket.stateTick = networkTick;

			m_healthUpdatePacket.entities.clear();
			m_healthUpdatePacket.layers.clear();

			for (auto it = m_layers.begin(); it != m_layers.end(); ++it)
			{
				auto& layer = *it.value();
				if (layer.healthUpdateEvents.empty())
					continue;

				LayerIndex layerIndex = it.key();

				auto& layerData = m_healthUpdatePacket.layers.emplace_back();
				layerData.layerIndex = layerIndex;
				layerData.entityCount = static_cast<Nz::UInt32>(layer.healthUpdateEvents.size());

				for (auto&& pair : layer.healthUpdateEvents)
				{
					auto& eventData = pair.second;
					
					auto& entityData = m_healthUpdatePacket.entities.emplace_back();
					entityData.id = pair.first;
					entityData.currentHealth = eventData.currentHealth;
				}
				layer.healthUpdateEvents.clear();
			}

			m_session.SendPacket(m_healthUpdatePacket);

			m_pendingEvents.Clear(VisibilityEventType::HealthUpdate);
		}

		if (m_pendingEvents.Test(VisibilityEventType::InputUpdate))
		{
			m_inputUpdatePacket.stateTick = networkTick;

			m_inputUpdatePacket.entities.clear();
			m_inputUpdatePacket.layers.clear();

			for (auto it = m_layers.begin(); it != m_layers.end(); ++it)
			{
				auto& layer = *it.value();
				if (layer.inputUpdateEvents.empty())
					continue;

				LayerIndex layerIndex = it.key();

				auto& layerData = m_inputUpdatePacket.layers.emplace_back();
				layerData.layerIndex = layerIndex;
				layerData.entityCount = static_cast<Nz::UInt32>(layer.inputUpdateEvents.size());

				for (auto&& pair : layer.inputUpdateEvents)
				{
					auto& eventData = pair.second;

					auto& entityData = m_inputUpdatePacket.entities.emplace_back();
					entityData.id = pair.first;
					entityData.inputs = eventData.inputs;
				}
				layer.inputUpdateEvents.clear();
			}

			m_session.SendPacket(m_inputUpdatePacket);

			m_pendingEvents.Clear(VisibilityEventType::InputUpdate);
		}

		if (m_pendingEvents.Test(VisibilityEventType::PlayAnimation))
		{
			m_entitiesAnimationPacket.stateTick = networkTick;

			m_entitiesAnimationPacket.entities.clear();
			m_entitiesAnimationPacket.layers.clear();

			for (auto it = m_layers.begin(); it != m_layers.end(); ++it)
			{
				auto& layer = *it.value();
				if (layer.playAnimationEvents.empty())
					continue;

				LayerIndex layerIndex = it.key();

				auto& layerData = m_entitiesAnimationPacket.layers.emplace_back();
				layerData.layerIndex = layerIndex;
				layerData.entityCount = static_cast<Nz::UInt32>(layer.playAnimationEvents.size());

				for (auto&& pair : layer.playAnimationEvents)
				{
					auto& eventData = pair.second;

					auto& entityData = m_entitiesAnimationPacket.entities.emplace_back();
					entityData.entityId = pair.first;
					entityData.animId = static_cast<Nz::UInt8>(eventData.animId);
				}
				layer.playAnimationEvents.clear();
			}

			m_session.SendPacket(m_entitiesAnimationPacket);

			m_pendingEvents.Clear(VisibilityEventType::PlayAnimation);
		}

		if (!m_layers.empty())
			SendMatchState();

		for (auto it = m_pendingEntitiesEvent.begin(); it != m_pendingEntitiesEvent.end();)
		{
			LayerIndex layerId = LayerIndex(it.key() >> 32);
			Nz::UInt32 entityId = Nz::UInt32(it.key() & 0xFFFFFFFF);

			if (m_layers.find(layerId) == m_layers.end())
			{
				// If a pending event is related to a layer which is no longer visible, drop it
				it = m_pendingEntitiesEvent.erase(it);
				continue;
			}
			Layer& layer = *m_layers[layerId];

			if (layer.visibleEntities.UnboundedTest(entityId))
			{
				auto& callbackVec = it.value();
				for (auto&& func : callbackVec)
					func();

				it = m_pendingEntitiesEvent.erase(it);
			}
			else
				++it;
		}

		for (auto it = m_multiplePendingEntitiesEvent.begin(); it != m_multiplePendingEntitiesEvent.end();)
		{
			if (m_layers.find(it->layerIndex) == m_layers.end())
			{
				// If a pending event is related to a layer which is no longer visible, drop it
				it = m_multiplePendingEntitiesEvent.erase(it);
				continue;
			}

			Layer& layer = *m_layers[it->layerIndex];

			m_tempBitset.PerformsAND(layer.visibleEntities, it->entitiesId);
			if (m_tempBitset == it->entitiesId)
			{
				it->sendFunction();

				it = m_multiplePendingEntitiesEvent.erase(it);
			}
			else
				++it;
		}
	}

	void MatchClientVisibility::HandleEntityCreation(LayerIndex layerIndex, const NetworkSyncSystem::EntityCreation& eventData)
	{
		assert(m_layers.find(layerIndex) != m_layers.end());
		Layer& layer = *m_layers[layerIndex];
		layer.creationEvents[eventData.entityId] = eventData;
		layer.visibleEntities.UnboundedSet(eventData.entityId);

		m_pendingEvents.Set(VisibilityEventType::Creation);
	}

	void MatchClientVisibility::HandleEntityRemove(LayerIndex layerIndex, Ndk::EntityId entityId, bool deathEvent)
	{
		assert(m_layers.find(layerIndex) != m_layers.end());
		Layer& layer = *m_layers[layerIndex];

		// Only send entity destruction packet if this entity was already created client-side
		auto it = layer.creationEvents.find(entityId);
		if (it != layer.creationEvents.end())
			layer.creationEvents.erase(it);
		else
		{
			if (deathEvent)
			{
				layer.deathEvents.insert(entityId);
				m_pendingEvents.Set(VisibilityEventType::Death);
			}
			else
			{
				layer.destructionEvents.insert(entityId);
				m_pendingEvents.Set(VisibilityEventType::Destruction);
			}
		}

		layer.inputUpdateEvents.erase(entityId);
		layer.healthUpdateEvents.erase(entityId);
		layer.playAnimationEvents.erase(entityId);
		layer.staticMovementUpdateEvents.erase(entityId);

		layer.visibleEntities.UnboundedReset(entityId);
	}

	void MatchClientVisibility::SendMatchState()
	{
		Terrain& terrain = m_match.GetTerrain();

		m_matchStatePacket.entities.clear();
		m_matchStatePacket.layers.clear();
		m_matchStatePacket.stateTick = m_match.GetNetworkTick();

		for (auto it = m_layers.begin(); it != m_layers.end(); ++it)
		{
			LayerIndex layerIndex = it.key();
			auto& layer = *it.value();

			std::size_t oldEntityCount = m_matchStatePacket.entities.size();

			for (auto&& pair : layer.staticMovementUpdateEvents)
				BuildMovementPacket(m_matchStatePacket.entities.emplace_back(), pair.second);

			layer.staticMovementUpdateEvents.clear();

			TerrainLayer& terrainLayer = terrain.GetLayer(layerIndex);
			const NetworkSyncSystem& syncSystem = terrainLayer.GetWorld().GetSystem<NetworkSyncSystem>();

			syncSystem.MoveEntities([&](const NetworkSyncSystem::EntityMovement* entitiesMovement, std::size_t entityCount)
			{
				for (std::size_t i = 0; i < entityCount; ++i)
					BuildMovementPacket(m_matchStatePacket.entities.emplace_back(), entitiesMovement[i]);
			});

			std::size_t entityCount = m_matchStatePacket.entities.size() - oldEntityCount;
			if (entityCount > 0)
			{
				auto& layerData = m_matchStatePacket.layers.emplace_back();
				layerData.layerIndex = layerIndex;
				layerData.entityCount = static_cast<Nz::UInt32>(entityCount);
			}
		}

		m_session.SendPacket(m_matchStatePacket);
	}

	void MatchClientVisibility::BuildMovementPacket(Packets::MatchState::Entity& packetData, const NetworkSyncSystem::EntityMovement& eventData)
	{
		packetData.id = eventData.entityId;
		packetData.position = eventData.position;
		packetData.rotation = eventData.rotation;

		if (eventData.playerMovement.has_value())
		{
			packetData.playerMovement.emplace();
			packetData.playerMovement->isFacingRight = eventData.playerMovement->isFacingRight;
		}

		if (eventData.physicsProperties.has_value())
		{
			packetData.physicsProperties.emplace();
			packetData.physicsProperties->angularVelocity = eventData.physicsProperties->angularVelocity;
			packetData.physicsProperties->linearVelocity = eventData.physicsProperties->linearVelocity;
		}
	}

	void MatchClientVisibility::FillEntityData(const NetworkSyncSystem::EntityCreation& creationEvent, Packets::Helper::EntityData& entityData)
	{
		const NetworkStringStore& networkStringStore = m_match.GetNetworkStringStore();

		assert(creationEvent.uniqueId > 0);

		entityData.entityClass = networkStringStore.CheckStringIndex(creationEvent.entityClass);
		entityData.uniqueId = static_cast<Nz::UInt64>(creationEvent.uniqueId);
		entityData.position = creationEvent.position;
		entityData.rotation = creationEvent.rotation;

		if (creationEvent.inputs.has_value())
			entityData.inputs = creationEvent.inputs.value();

		if (creationEvent.parent.has_value())
			entityData.parentId = creationEvent.parent.value();

		if (creationEvent.healthProperties.has_value())
		{
			entityData.health.emplace();
			entityData.health->currentHealth = creationEvent.healthProperties->currentHealth;
			entityData.health->maxHealth = creationEvent.healthProperties->maxHealth;
		}

		if (creationEvent.name.has_value())
			entityData.name.emplace(creationEvent.name.value());

		if (creationEvent.playerMovement.has_value())
		{
			entityData.playerMovement.emplace();
			entityData.playerMovement->isFacingRight = creationEvent.playerMovement->isFacingRight;
		}

		if (creationEvent.physicsProperties.has_value())
		{
			entityData.physicsProperties.emplace();
			entityData.physicsProperties->angularVelocity = creationEvent.physicsProperties->angularVelocity;
			entityData.physicsProperties->linearVelocity = creationEvent.physicsProperties->linearVelocity;
		}

		for (auto&& [propertyName, propertyValue] : creationEvent.properties)
		{
			auto& propertyData = entityData.properties.emplace_back();
			propertyData.name = networkStringStore.CheckStringIndex(propertyName);

			std::visit([&](auto&& propertyValue)
			{
				using T = std::decay_t<decltype(propertyValue)>;
				constexpr bool IsArray = IsSameTpl_v<EntityPropertyArray, T>;
				using PropertyType = std::conditional_t<IsArray, typename IsSameTpl<EntityPropertyArray, T>::ContainedType, T>;

				propertyData.isArray = IsArray;

				auto& vec = propertyData.value.emplace<std::vector<PropertyType>>();

				if constexpr (IsArray)
				{
					std::size_t elementCount = propertyValue.GetSize();
					vec.reserve(elementCount);

					for (std::size_t i = 0; i < elementCount; ++i)
						vec.emplace_back(std::move(propertyValue[i]));
				}
				else
					vec.push_back(propertyValue);

			}, std::move(propertyValue));
		}
	}
}
