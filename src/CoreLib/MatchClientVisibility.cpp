// Copyright (C) 2020 Jérôme Leclercq
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
		m_newlyHiddenLayers.UnboundedReset(layerIndex);

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

				if (layer.visibleEntities.find(entityMovement.entityId) == layer.visibleEntities.end())
					return;

				layer.staticMovementUpdateEvents[entityMovement.entityId] = entityMovement;
			});

			layer.onEntityPlayAnimation.Connect(syncSystem.OnEntityPlayAnimation, [this, layerIndex](NetworkSyncSystem*, const NetworkSyncSystem::EntityPlayAnimation& entityPlayAnimation)
			{
				assert(m_layers.find(layerIndex) != m_layers.end());
				Layer& layer = *m_layers[layerIndex];

				if (layer.visibleEntities.find(entityPlayAnimation.entityId) == layer.visibleEntities.end())
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
					if (layer.visibleEntities.find(events[i].entityId) == layer.visibleEntities.end())
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

					if (layer.visibleEntities.find(events[i].entityId) == layer.visibleEntities.end())
						continue;

					layer.inputUpdateEvents[events[i].entityId] = events[i];
					m_pendingEvents.Set(VisibilityEventType::InputUpdate);
				}
			});

			layer.onEntitiesPhysicsUpdate.Connect(syncSystem.OnEntitiesPhysicsUpdate, [this, layerIndex](NetworkSyncSystem*, const NetworkSyncSystem::EntityPhysics* events, std::size_t entityCount)
			{
				assert(m_layers.find(layerIndex) != m_layers.end());
				Layer& layer = *m_layers[layerIndex];

				for (std::size_t i = 0; i < entityCount; ++i)
				{
					if (layer.visibleEntities.find(events[i].entityId) == layer.visibleEntities.end())
						continue;

					layer.physicsEvents[events[i].entityId] = events[i];
					m_pendingEvents.Set(VisibilityEventType::PhysicsUpdate);
				}
			});
			
			layer.onEntitiesScaleUpdate.Connect(syncSystem.OnEntitiesScaleUpdate, [this, layerIndex](NetworkSyncSystem*, const NetworkSyncSystem::EntityScale* events, std::size_t entityCount)
			{
				assert(m_layers.find(layerIndex) != m_layers.end());
				Layer& layer = *m_layers[layerIndex];

				for (std::size_t i = 0; i < entityCount; ++i)
				{
					if (layer.visibleEntities.find(events[i].entityId) == layer.visibleEntities.end())
						continue;

					layer.scaleEvents[events[i].entityId] = events[i];
					m_pendingEvents.Set(VisibilityEventType::ScaleUpdate);
				}
			});

			layer.onEntitiesWeaponUpdate.Connect(syncSystem.OnEntitiesWeaponUpdate, [this, layerIndex](NetworkSyncSystem*, const NetworkSyncSystem::EntityWeapon* events, std::size_t entityCount)
			{
				assert(m_layers.find(layerIndex) != m_layers.end());
				Layer& layer = *m_layers[layerIndex];

				for (std::size_t i = 0; i < entityCount; ++i)
				{
					if (layer.visibleEntities.find(events[i].entityId) == layer.visibleEntities.end())
						continue;

					layer.weaponEvents[events[i].entityId] = events[i];
					m_pendingEvents.Set(VisibilityEventType::WeaponUpdate);
				}
			});

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
						layer.visibleEntities.emplace(entity->GetId(), Layer::VisibleEntityData{});

					continue;
				}

				syncSystem.CreateEntities([&](const NetworkSyncSystem::EntityCreation* entitiesCreation, std::size_t entityCount)
				{
					for (std::size_t i = 0; i < entityCount; ++i)
					{
						if (layer.visibleEntities.find(entitiesCreation[i].entityId) == layer.visibleEntities.end())
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

					if (eventData->weapon)
					{
						NetworkSyncSystem::EntityWeapon weaponEvent;
						weaponEvent.entityId = eventData->entityId;
						weaponEvent.weaponId = eventData->weapon.value();

						layer.weaponEvents[weaponEvent.entityId] = weaponEvent;
						m_pendingEvents.Set(VisibilityEventType::WeaponUpdate);
					}

					for (auto&& [layerIndex, entityIndex] : eventData->dependentIds)
						HandleDependentEntity(static_cast<Nz::UInt32>(entityIndex));

					auto& entityData = enableLayerPacket.layerEntities.emplace_back();
					entityData.id = eventData->entityId;
					FillEntityData(eventData.value(), entityData.data);

					layer.visibleEntities.emplace(entityId, Layer::VisibleEntityData{});

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

					if (eventData->weapon)
					{
						NetworkSyncSystem::EntityWeapon weaponEvent;
						weaponEvent.entityId = eventData->entityId;
						weaponEvent.weaponId = eventData->weapon.value();

						layer.weaponEvents[weaponEvent.entityId] = weaponEvent;
						m_pendingEvents.Set(VisibilityEventType::WeaponUpdate);
					}

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

		if (m_pendingEvents.Test(VisibilityEventType::PhysicsUpdate))
		{
			for (auto it = m_layers.begin(); it != m_layers.end(); ++it)
			{
				auto& layer = *it.value();
				if (layer.physicsEvents.empty())
					continue;

				LayerIndex layerIndex = it.key();

				for (auto&& pair : layer.physicsEvents)
				{
					Packets::EntityPhysics physicsPacket;
					physicsPacket.entityId.layerId = layerIndex;
					physicsPacket.entityId.entityId = pair.first;
					physicsPacket.stateTick = networkTick;

					auto& physicsData = pair.second;
					physicsPacket.asleep = physicsData.isAsleep;
					physicsPacket.mass = physicsData.mass;
					physicsPacket.momentOfInertia = physicsData.momentOfInertia;
					
					if (physicsData.playerMovement)
					{
						const auto& playerMovementData = physicsData.playerMovement.value();

						auto& packetMovement = physicsPacket.playerMovement.emplace();
						packetMovement.jumpHeight = playerMovementData.jumpHeight;
						packetMovement.jumpHeightBoost = playerMovementData.jumpHeightBoost;
						packetMovement.movementSpeed = playerMovementData.movementSpeed;
					}

					m_session.SendPacket(physicsPacket);
				}

				layer.physicsEvents.clear();
			}

			m_pendingEvents.Clear(VisibilityEventType::PhysicsUpdate);
		}

		if (m_pendingEvents.Test(VisibilityEventType::ScaleUpdate))
		{
			for (auto it = m_layers.begin(); it != m_layers.end(); ++it)
			{
				auto& layer = *it.value();
				if (layer.scaleEvents.empty())
					continue;

				LayerIndex layerIndex = it.key();

				for (auto&& pair : layer.scaleEvents)
				{
					Packets::EntityScale scalePacket;
					scalePacket.entityId.layerId = layerIndex;
					scalePacket.entityId.entityId = pair.first;
					scalePacket.stateTick = networkTick;
					scalePacket.newScale = pair.second.newScale;

					m_session.SendPacket(scalePacket);
				}

				layer.scaleEvents.clear();
			}

			m_pendingEvents.Clear(VisibilityEventType::ScaleUpdate);
		}

		if (m_pendingEvents.Test(VisibilityEventType::WeaponUpdate))
		{
			for (auto it = m_layers.begin(); it != m_layers.end(); ++it)
			{
				auto& layer = *it.value();
				if (layer.weaponEvents.empty())
					continue;

				LayerIndex layerIndex = it.key();

				for (auto&& pair : layer.weaponEvents)
				{
					Packets::EntityWeapon weaponPacket;
					weaponPacket.entityId.layerId = layerIndex;
					weaponPacket.entityId.entityId = pair.first;
					weaponPacket.stateTick = networkTick;

					auto& weaponData = pair.second;
					weaponPacket.weaponEntityId = (weaponData.weaponId.has_value()) ? weaponData.weaponId.value() : Packets::EntityWeapon::NoWeapon;

					m_session.SendPacket(weaponPacket);
				}

				layer.weaponEvents.clear();
			}

			m_pendingEvents.Clear(VisibilityEventType::WeaponUpdate);
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

			if (layer.visibleEntities.find(entityId) != layer.visibleEntities.end())
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

			bool allEntitiesVisible = true;
			for (std::size_t i = it->entitiesId.FindFirst(); i != it->entitiesId.npos; i = it->entitiesId.FindNext(i))
			{
				if (layer.visibleEntities.find(Nz::UInt32(i)) == layer.visibleEntities.end())
				{
					allEntitiesVisible = false;
					break;
				}
			}

			if (allEntitiesVisible)
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
		layer.visibleEntities.emplace(eventData.entityId, Layer::VisibleEntityData{});

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
		layer.physicsEvents.erase(entityId);
		layer.playAnimationEvents.erase(entityId);
		layer.staticMovementUpdateEvents.erase(entityId);
		layer.visibleEntities.erase(entityId);
		layer.weaponEvents.erase(entityId);
	}

	void MatchClientVisibility::SendMatchState()
	{
		constexpr std::size_t MaxPacketSize = Nz::ENetConstants::ENetHost_DefaultMTU - sizeof(Nz::ENetProtocolHeader) - sizeof(Nz::ENetProtocolSendFragment);

		auto HasExceededPacketSize = [&]() -> bool
		{
			std::size_t size = Packets::EstimateSize(m_matchStatePacket);
			return size > MaxPacketSize;
		};

		Terrain& terrain = m_match.GetTerrain();

		m_priorityMovementData.clear();
		auto PushMovementData = [this](LayerIndex layerIndex, Nz::UInt8 priorityAccumulator, const NetworkSyncSystem::EntityMovement& movementData, bool isStatic)
		{
			m_priorityMovementData.push_back(PriorityMovementData{
				priorityAccumulator,
				layerIndex,
				movementData,
				isStatic
			});
		};

		for (auto it = m_layers.begin(); it != m_layers.end(); ++it)
		{
			LayerIndex layerIndex = it.key();
			auto& layer = *it.value();

			for (auto&& pair : layer.staticMovementUpdateEvents)
			{
				auto visibleIt = layer.visibleEntities.find(pair.first);
				assert(visibleIt != layer.visibleEntities.end());

				auto& visibleData = visibleIt.value();
				visibleData.priorityAccumulator += 3; //< TODO use NetworkSyncComponent value

				PushMovementData(layerIndex, visibleData.priorityAccumulator, pair.second, true);
			}

			layer.staticMovementUpdateEvents.clear();

			TerrainLayer& terrainLayer = terrain.GetLayer(layerIndex);
			const NetworkSyncSystem& syncSystem = terrainLayer.GetWorld().GetSystem<NetworkSyncSystem>();

			syncSystem.MoveEntities([&](const NetworkSyncSystem::EntityMovement* entitiesMovement, std::size_t entityCount)
			{
				for (std::size_t i = 0; i < entityCount; ++i)
				{
					auto& movementData = entitiesMovement[i];

					auto visibleIt = layer.visibleEntities.find(movementData.entityId);
					if (visibleIt == layer.visibleEntities.end())
						continue;

					auto& visibleData = visibleIt.value();
					Nz::UInt64 entityKey = Nz::UInt64(layerIndex) << 32 | movementData.entityId;
					if (m_controlledEntities.find(entityKey) != m_controlledEntities.end())
					{
						//FIXME
						visibleData.priorityAccumulator = 0xFF;
					}
					else
						visibleData.priorityAccumulator += 1; //< TODO use NetworkSyncComponent value

					PushMovementData(layerIndex, visibleData.priorityAccumulator, movementData, false);
				}
			});
		}

		std::sort(m_priorityMovementData.begin(), m_priorityMovementData.end(), [](const PriorityMovementData& lhs, const PriorityMovementData& rhs)
		{
			return lhs.priorityAccumulator > rhs.priorityAccumulator;
		});

		m_matchStatePacket.entities.clear();
		m_matchStatePacket.layers.clear();
		m_matchStatePacket.stateTick = m_match.GetNetworkTick();
		m_matchStatePacket.lastInputTick = m_session.GetLastInputTick();

		for (PriorityMovementData& movementData : m_priorityMovementData)
		{
			std::size_t entityIndex = 0;

			std::size_t layerIndex = 0;
			std::size_t layerCount = m_matchStatePacket.layers.size();
			for (; layerIndex < layerCount; ++layerIndex)
			{
				auto& layer = m_matchStatePacket.layers[layerIndex];
				entityIndex += layer.entityCount;

				if (layer.layerIndex == movementData.layerIndex)
				{
					layer.entityCount++;
					break;
				}
			}

			// Has layer been found?
			if (layerIndex == layerCount)
			{
				// No, insert it as a new one
				auto& layer = m_matchStatePacket.layers.emplace_back();
				layer.entityCount = 1;
				layer.layerIndex = movementData.layerIndex;
			}

			assert(entityIndex <= m_matchStatePacket.entities.size());
			auto entityIt = m_matchStatePacket.entities.emplace(m_matchStatePacket.entities.begin() + entityIndex);
			BuildMovementPacket(*entityIt, movementData.movementData);

			if (HasExceededPacketSize())
			{
				// Remove last inserted entity
				m_matchStatePacket.entities.erase(entityIt);

				auto& layer = m_matchStatePacket.layers[layerIndex];
				if (--layer.entityCount == 0)
				{
					assert(layerIndex == layerCount);
					m_matchStatePacket.layers.pop_back();
				}

				break;
			}

			auto layerIt = m_layers.find(movementData.layerIndex);
			assert(layerIt != m_layers.end());

			auto& layerData = *layerIt.value();

			Nz::UInt32 entityId = Nz::UInt32(movementData.movementData.entityId);

			auto visibleIt = layerData.visibleEntities.find(entityId);
			assert(visibleIt != layerData.visibleEntities.end());

			auto& visibleData = visibleIt.value();
			visibleData.priorityAccumulator = 0;

			if (movementData.staticEntity)
				layerData.staticMovementUpdateEvents.erase(entityId);
		}

		//bwLog(m_match.GetLogger(), LogLevel::Debug, "Entity count: {0} (packet size: {1})", m_matchStatePacket.entities.size(), Packets::EstimateSize(m_matchStatePacket));

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

		if (!Nz::NumberEquals(creationEvent.scale, 1.f))
			entityData.scale = creationEvent.scale;

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

		if (!creationEvent.name.empty())
			entityData.name = creationEvent.name;

		if (creationEvent.playerMovement.has_value())
		{
			entityData.playerMovement.emplace();
			entityData.playerMovement->isFacingRight = creationEvent.playerMovement->isFacingRight;
		}

		if (creationEvent.physicsProperties.has_value())
		{
			const auto& physicsProperties = *creationEvent.physicsProperties;

			entityData.physicsProperties.emplace();
			entityData.physicsProperties->angularVelocity = physicsProperties.angularVelocity;
			entityData.physicsProperties->linearVelocity = physicsProperties.linearVelocity;
			entityData.physicsProperties->isAsleep = physicsProperties.isSleeping;
			entityData.physicsProperties->mass = physicsProperties.mass;
			entityData.physicsProperties->momentOfInertia = physicsProperties.momentOfInertia;
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
