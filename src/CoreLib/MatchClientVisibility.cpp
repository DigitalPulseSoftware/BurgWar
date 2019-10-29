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
	void MatchClientVisibility::Update()
	{
		// Handle hidden and shown layers
		if (m_newlyHiddenLayers.GetSize() != 0)
		{
			for (std::size_t layerIndex = m_newlyHiddenLayers.FindFirst(); layerIndex != m_newlyHiddenLayers.npos; layerIndex = m_newlyHiddenLayers.FindNext(layerIndex))
			{
				Terrain& terrain = m_match.GetTerrain();
				assert(layerIndex < terrain.GetLayerCount());
				TerrainLayer& layer = terrain.GetLayer(layerIndex);

				/* Delete all visible entities on that layer */

				const NetworkSyncSystem& syncSystem = layer.GetWorld().GetSystem<NetworkSyncSystem>();

				syncSystem.DeleteEntities([&](const NetworkSyncSystem::EntityDestruction* entitiesDestruction, std::size_t entityCount)
					{
						for (std::size_t i = 0; i < entityCount; ++i)
							HandleEntityDestruction(layerIndex, entitiesDestruction[i], true);
					});

				auto it = m_layers.find(layerIndex);
				assert(it != m_layers.end());
				m_layers.erase(it);

				m_visibleLayers.Reset(layerIndex);
			}
			m_newlyHiddenLayers.Clear();
		}

		if (m_newlyVisibleLayers.GetSize() != 0)
		{
			for (std::size_t layerIndex = m_newlyVisibleLayers.FindFirst(); layerIndex != m_newlyVisibleLayers.npos; layerIndex = m_newlyVisibleLayers.FindNext(layerIndex))
			{
				Terrain& terrain = m_match.GetTerrain();
				assert(layerIndex < terrain.GetLayerCount());

				/* Create all newly visible entities */
				TerrainLayer& terrainLayer = terrain.GetLayer(layerIndex);
				NetworkSyncSystem& syncSystem = terrainLayer.GetWorld().GetSystem<NetworkSyncSystem>();

				assert(m_layers.find(layerIndex) == m_layers.end());
				Layer& layer = m_layers[layerIndex];

				m_visibleLayers.UnboundedSet(layerIndex);

				layer.onEntityCreatedSlot.Connect(syncSystem.OnEntityCreated, [this](NetworkSyncSystem* syncSystem, const NetworkSyncSystem::EntityCreation& entityCreation)
				{
					HandleEntityCreation(syncSystem->GetLayerIndex(), entityCreation);
				});

				layer.onEntityDeletedSlot.Connect(syncSystem.OnEntityDeleted, [this](NetworkSyncSystem* syncSystem, const NetworkSyncSystem::EntityDestruction& entityDestruction)
				{
					HandleEntityDestruction(syncSystem->GetLayerIndex(), entityDestruction, false);
				});

				layer.onEntityInvalidated.Connect(syncSystem.OnEntityInvalidated, [this, layerIndex](NetworkSyncSystem*, const NetworkSyncSystem::EntityMovement& entityMovement)
				{
					assert(m_layers.find(layerIndex) != m_layers.end());
					Layer& layer = m_layers[layerIndex];

					if (!layer.visibleEntities.UnboundedTest(entityMovement.entityId))
						return;

					m_staticMovementUpdateEvents[entityMovement.entityId] = entityMovement;
				});

				layer.onEntityPlayAnimation.Connect(syncSystem.OnEntityPlayAnimation, [this, layerIndex](NetworkSyncSystem*, const NetworkSyncSystem::EntityPlayAnimation& entityPlayAnimation)
				{
					assert(m_layers.find(layerIndex) != m_layers.end());
					Layer& layer = m_layers[layerIndex];

					if (!layer.visibleEntities.UnboundedTest(entityPlayAnimation.entityId))
						return;

					m_playAnimationEvents[entityPlayAnimation.entityId] = entityPlayAnimation;
				});

				layer.onEntitiesDeath.Connect(syncSystem.OnEntitiesDeath, [this, layerIndex](NetworkSyncSystem*, const Ndk::EntityId* entityIds, std::size_t entityCount)
				{
					assert(m_layers.find(layerIndex) != m_layers.end());
					Layer& layer = m_layers[layerIndex];
				
					for (std::size_t i = 0; i < entityCount; ++i)
					{
						if (!layer.visibleEntities.UnboundedTest(entityIds[i]))
							return;

						m_deathEvents.emplace(entityIds[i]);
					}
				});

				layer.onEntitiesHealthUpdate.Connect(syncSystem.OnEntitiesHealthUpdate, [this, layerIndex](NetworkSyncSystem*, const NetworkSyncSystem::EntityHealth* events, std::size_t entityCount)
				{
					assert(m_layers.find(layerIndex) != m_layers.end());
					Layer& layer = m_layers[layerIndex];
				
					for (std::size_t i = 0; i < entityCount; ++i)
					{
						if (!layer.visibleEntities.UnboundedTest(events[i].entityId))
							return;

						m_healthUpdateEvents[events[i].entityId] = events[i];
					}
				});

				layer.onEntitiesInputUpdate.Connect(syncSystem.OnEntitiesInputUpdate, [this, layerIndex](NetworkSyncSystem*, const NetworkSyncSystem::EntityInputs* events, std::size_t entityCount)
				{
					assert(m_layers.find(layerIndex) != m_layers.end());
					Layer& layer = m_layers[layerIndex];

					for (std::size_t i = 0; i < entityCount; ++i)
					{
						if (!layer.visibleEntities.UnboundedTest(events[i].entityId))
							return;

						m_inputUpdateEvents[BuildEntityId(layerIndex, events[i].entityId)] = events[i];
					}
				});

				syncSystem.CreateEntities([&](const NetworkSyncSystem::EntityCreation* entitiesCreation, std::size_t entityCount)
				{
					for (std::size_t i = 0; i < entityCount; ++i)
						HandleEntityCreation(layerIndex, entitiesCreation[i]);
				});
			}
			m_newlyVisibleLayers.Clear();
		}

		// Send packet in fixed order
		if (!m_deathEvents.empty())
		{
			m_entitiesDeathPacket.stateTick = m_match.GetNetworkTick();

			m_entitiesDeathPacket.entities.clear();
			for (Nz::UInt64 entityId : m_deathEvents)
			{
				auto& entityData = m_entitiesDeathPacket.entities.emplace_back();
				entityData.id = DecodeEntityId(entityId);
			}
			m_deathEvents.clear();

			m_session.SendPacket(m_entitiesDeathPacket);
		}

		if (!m_destructionEvents.empty())
		{
			m_deleteEntitiesPacket.stateTick = m_match.GetNetworkTick();

			m_deleteEntitiesPacket.entities.clear();
			for (Nz::UInt64 entityId : m_destructionEvents)
			{
				auto& entityData = m_deleteEntitiesPacket.entities.emplace_back();
				entityData.id = DecodeEntityId(entityId);
			}
			m_destructionEvents.clear();

			m_session.SendPacket(m_deleteEntitiesPacket);
		}

		if (!m_creationEvents.empty())
		{
			// Handle parents
			m_createEntitiesPacket.entities.clear();
			std::function<void(PendingCreationEventMap::iterator it)> PushEntity;
			PushEntity = [&](PendingCreationEventMap::iterator it)
			{
				auto& entityId = it.key();
				auto& eventData = it.value();
				if (!eventData.has_value())
					return;

				Nz::UInt16 layerIndex = Nz::UInt16(entityId >> 32);

				if (eventData->parent)
				{
					Nz::UInt32 parentId = static_cast<Nz::UInt32>(eventData->parent.value());
					auto it = m_creationEvents.find(BuildEntityId(layerIndex, parentId));
					if (it != m_creationEvents.end())
						PushEntity(it);
				}

				const NetworkStringStore& networkStringStore = m_match.GetNetworkStringStore();

				m_createEntitiesPacket.stateTick = m_match.GetNetworkTick();
				auto& entityData = m_createEntitiesPacket.entities.emplace_back();
				entityData.id.layerId = layerIndex;
				entityData.id.entityId = eventData->entityId;
				entityData.entityClass = networkStringStore.CheckStringIndex(eventData->entityClass);
				entityData.position = eventData->position;
				entityData.rotation = eventData->rotation;

				if (eventData->inputs.has_value())
					entityData.inputs = eventData->inputs.value();

				if (eventData->parent.has_value())
					entityData.parentId = eventData->parent.value();

				if (eventData->healthProperties.has_value())
				{
					entityData.health.emplace();
					entityData.health->currentHealth = eventData->healthProperties->currentHealth;
					entityData.health->maxHealth = eventData->healthProperties->maxHealth;
				}

				if (eventData->name.has_value())
					entityData.name.emplace(eventData->name.value());

				if (eventData->playerMovement.has_value())
				{
					entityData.playerMovement.emplace();
					entityData.playerMovement->isFacingRight = eventData->playerMovement->isFacingRight;
				}

				if (eventData->physicsProperties.has_value())
				{
					entityData.physicsProperties.emplace();
					entityData.physicsProperties->angularVelocity = eventData->physicsProperties->angularVelocity;
					entityData.physicsProperties->linearVelocity = eventData->physicsProperties->linearVelocity;
				}

				for (auto&& [propertyName, propertyValue] : eventData->properties)
				{
					auto& propertyData = entityData.properties.emplace_back();
					propertyData.name = networkStringStore.CheckStringIndex(propertyName);

					std::visit([&](auto&& propertyValue)
					{
						using T = std::decay_t<decltype(propertyValue)>;
						constexpr bool IsArray = IsSameTpl_v<EntityPropertyArray, T>;
						using PropertyType = std::conditional_t<IsArray, typename IsSameTpl<EntityPropertyArray, T>::ContainedType, T>;

						if constexpr (std::is_same_v<PropertyType, bool> ||
						              std::is_same_v<PropertyType, float> ||
						              std::is_same_v<PropertyType, Nz::Int64> ||
						              std::is_same_v<PropertyType, std::string> ||
						              std::is_same_v<PropertyType, Nz::Vector2f> ||
						              std::is_same_v<PropertyType, Nz::Vector2i64>)

						{
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
						}
						else
							static_assert(AlwaysFalse<PropertyType>::value, "non-exhaustive visitor");

					}, std::move(propertyValue));
				}
				
				eventData.reset();
			};

			for (auto it = m_creationEvents.begin(); it != m_creationEvents.end(); ++it)
				PushEntity(it);
			m_creationEvents.clear();

			m_session.SendPacket(m_createEntitiesPacket);
		}

		if (!m_healthUpdateEvents.empty())
		{
			m_healthUpdatePacket.stateTick = m_match.GetNetworkTick();
			m_healthUpdatePacket.entities.clear();

			for (auto&& pair : m_healthUpdateEvents)
			{
				auto& eventData = pair.second;

				auto& entityData = m_healthUpdatePacket.entities.emplace_back();
				entityData.currentHealth = eventData.currentHealth;
				entityData.id = DecodeEntityId(pair.first);
			}
			m_healthUpdateEvents.clear();

			m_session.SendPacket(m_healthUpdatePacket);
		}

		if (!m_inputUpdateEvents.empty())
		{
			m_inputUpdatePacket.stateTick = m_match.GetNetworkTick();
			m_inputUpdatePacket.entities.clear();

			for (auto&& pair : m_inputUpdateEvents)
			{
				auto& eventData = pair.second;

				auto& entityData = m_inputUpdatePacket.entities.emplace_back();
				entityData.id = DecodeEntityId(pair.first);
				entityData.inputs = eventData.inputs;
			}
			m_inputUpdateEvents.clear();

			m_session.SendPacket(m_inputUpdatePacket);
		}

		if (!m_playAnimationEvents.empty())
		{
			m_entitiesAnimationPacket.stateTick = m_match.GetNetworkTick();
			m_entitiesAnimationPacket.entities.clear();

			for (auto&& pair : m_playAnimationEvents)
			{
				auto& eventData = pair.second;

				auto& animData = m_entitiesAnimationPacket.entities.emplace_back();
				animData.animId = static_cast<Nz::UInt8>(eventData.animId);
				animData.entityId = DecodeEntityId(pair.first);
			}
			m_playAnimationEvents.clear();

			m_session.SendPacket(m_entitiesAnimationPacket);
		}

		if (!m_layers.empty())
			SendMatchState();

		for (auto it = m_pendingEntitiesEvent.begin(); it != m_pendingEntitiesEvent.end();)
		{
			auto entityId = DecodeEntityId(it.key());

			assert(m_layers.find(entityId.layerId) != m_layers.end());
			Layer& layer = m_layers[entityId.layerId];

			if (layer.visibleEntities.UnboundedTest(entityId.entityId))
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

			Layer& layer = m_layers[it->layerIndex];

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

	void MatchClientVisibility::HandleEntityCreation(Nz::UInt16 layerIndex, const NetworkSyncSystem::EntityCreation& eventData)
	{
		m_creationEvents[BuildEntityId(layerIndex, eventData.entityId)] = eventData;

		assert(m_layers.find(layerIndex) != m_layers.end());
		Layer& layer = m_layers[layerIndex];

		layer.visibleEntities.UnboundedSet(eventData.entityId);
	}

	void MatchClientVisibility::HandleEntityDestruction(Nz::UInt16 layerIndex, const NetworkSyncSystem::EntityDestruction& eventData, bool clearDeath)
	{
		Nz::UInt64 entityId = BuildEntityId(layerIndex, eventData.entityId);

		// Only send entity destruction packet if this entity was already created client-side
		auto it = m_creationEvents.find(entityId);
		if (it != m_creationEvents.end())
			m_creationEvents.erase(it);
		else
			m_destructionEvents.insert(entityId);

		m_inputUpdateEvents.erase(entityId);
		m_healthUpdateEvents.erase(entityId);
		m_playAnimationEvents.erase(entityId);
		m_staticMovementUpdateEvents.erase(entityId);

		if (clearDeath)
			m_deathEvents.erase(entityId);

		assert(m_layers.find(layerIndex) != m_layers.end());
		Layer& layer = m_layers[layerIndex];

		layer.visibleEntities.UnboundedReset(eventData.entityId);
	}

	void MatchClientVisibility::SendMatchState()
	{
		Terrain& terrain = m_match.GetTerrain();

		m_matchStatePacket.entities.clear();
		m_matchStatePacket.stateTick = m_match.GetNetworkTick();

		for (auto&& pair : m_staticMovementUpdateEvents)
			BuildMovementPacket(pair.first >> 16, m_matchStatePacket.entities.emplace_back(), pair.second);

		m_staticMovementUpdateEvents.clear();
		for (auto&& [layerIndex, layerData] : m_layers)
		{
			TerrainLayer& layer = terrain.GetLayer(layerIndex);
			const NetworkSyncSystem& syncSystem = layer.GetWorld().GetSystem<NetworkSyncSystem>();

			syncSystem.MoveEntities([&](const NetworkSyncSystem::EntityMovement* entitiesMovement, std::size_t entityCount)
			{
				for (std::size_t i = 0; i < entityCount; ++i)
					BuildMovementPacket(layerIndex, m_matchStatePacket.entities.emplace_back(), entitiesMovement[i]);
			});
		}

		m_session.SendPacket(m_matchStatePacket);
	}

	void MatchClientVisibility::BuildMovementPacket(Nz::UInt16 layerIndex, Packets::MatchState::Entity& packetData, const NetworkSyncSystem::EntityMovement& eventData)
	{
		packetData.id.layerId = layerIndex;
		packetData.id.entityId = eventData.entityId;
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
}
