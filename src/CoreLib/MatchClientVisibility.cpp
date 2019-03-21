// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/MatchClientVisibility.hpp>
#include <Nazara/Core/StackArray.hpp>
#include <Nazara/Core/StackVector.hpp>
#include <CoreLib/Protocol/Packets.hpp>
#include <CoreLib/Match.hpp>
#include <CoreLib/MatchClientSession.hpp>
#include <CoreLib/Terrain.hpp>
#include <cassert>
#include <queue>

namespace bw
{
	void MatchClientVisibility::Update(float elapsedTime)
	{
		// Send packet in fixed order
		if (!m_destructionEvents.empty())
		{
			m_deleteEntitiesPacket.entities.clear();
			for (Nz::UInt32 entityId : m_destructionEvents)
			{
				auto& entityData = m_deleteEntitiesPacket.entities.emplace_back();
				entityData.id = entityId;
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
				auto& eventData = it.value();
				if (!eventData.has_value())
					return;

				if (eventData->parent)
				{
					Nz::UInt32 parentId = static_cast<Nz::UInt32>(eventData->parent.value());
					auto it = m_creationEvents.find(parentId);
					if (it != m_creationEvents.end())
						PushEntity(it);
				}

				const NetworkStringStore& networkStringStore = m_match.GetNetworkStringStore();

				auto& entityData = m_createEntitiesPacket.entities.emplace_back();
				entityData.id = eventData->entityId;
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
						using PropertyType = std::conditional_t<IsArray, IsSameTpl<EntityPropertyArray, T>::ContainedType, T>;

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
			m_healthUpdatePacket.entities.clear();

			for (auto&& pair : m_healthUpdateEvents)
			{
				auto& eventData = pair.second;

				auto& entityData = m_healthUpdatePacket.entities.emplace_back();
				entityData.currentHealth = eventData.currentHealth;
				entityData.id = eventData.entityId;
			}
			m_healthUpdateEvents.clear();

			m_session.SendPacket(m_healthUpdatePacket);
		}

		if (!m_inputUpdateEvents.empty())
		{
			m_inputUpdatePacket.entities.clear();

			for (auto&& pair : m_inputUpdateEvents)
			{
				auto& eventData = pair.second;

				auto& entityData = m_inputUpdatePacket.entities.emplace_back();
				entityData.id = eventData.entityId;
				entityData.inputs = eventData.inputs;
			}
			m_inputUpdateEvents.clear();

			m_session.SendPacket(m_inputUpdatePacket);
		}

		if (!m_playAnimationEvents.empty())
		{
			for (auto&& pair : m_playAnimationEvents)
			{
				auto& eventData = pair.second;

				Packets::PlayAnimation animPacket;
				animPacket.animId = static_cast<Nz::UInt8>(eventData.animId);
				animPacket.entityId = static_cast<Nz::UInt32>(eventData.entityId);

				m_session.SendPacket(animPacket);
			}

			m_playAnimationEvents.clear();
		}

		m_entityMovementSendTimer += elapsedTime;
		if (m_entityMovementSendTimer >= m_entityMovementSendInterval)
		{
			m_entityMovementSendTimer -= m_entityMovementSendInterval;

			if (m_activeLayer != NoLayer)
				SendMatchState(elapsedTime);
		}

		for (auto it = m_pendingEntitiesEvent.begin(); it != m_pendingEntitiesEvent.end();)
		{
			if (m_visibleEntities.UnboundedTest(it.key()))
			{
				auto& callbackVec = it.value();
				for (auto&& func : callbackVec)
					func();

				it = m_pendingEntitiesEvent.erase(it);
			}
			else
				++it;
		}
	}

	void MatchClientVisibility::UpdateLayer(std::size_t layerIndex)
	{
		if (m_activeLayer == layerIndex)
			return;

		Terrain& terrain = m_match.GetTerrain();
		assert(layerIndex == NoLayer || layerIndex < terrain.GetLayerCount());

		if (m_activeLayer != NoLayer)
		{
			/* Delete all visible entities */

			TerrainLayer& layer = terrain.GetLayer(m_activeLayer);
			const NetworkSyncSystem& syncSystem = layer.GetWorld().GetSystem<NetworkSyncSystem>();

			syncSystem.DeleteEntities([&](const NetworkSyncSystem::EntityDestruction* entitiesDestruction, std::size_t entityCount)
			{
				for (std::size_t i = 0; i < entityCount; ++i)
					HandleEntityDestruction(entitiesDestruction[i]);
			});

			m_onEntityCreatedSlot.Disconnect();
			m_onEntityDeletedSlot.Disconnect();
			m_onEntityPlayAnimation.Disconnect();
			m_onEntitiesHealthUpdate.Disconnect();
			m_onEntitiesInputUpdate.Disconnect();
		}

		m_activeLayer = layerIndex;
		if (m_activeLayer != NoLayer)
		{
			/* Create all newly visible entities */
			TerrainLayer& layer = terrain.GetLayer(m_activeLayer);
			NetworkSyncSystem& syncSystem = layer.GetWorld().GetSystem<NetworkSyncSystem>();

			m_onEntityCreatedSlot.Connect(syncSystem.OnEntityCreated, [this](NetworkSyncSystem*, const NetworkSyncSystem::EntityCreation& entityCreation)
			{
				HandleEntityCreation(entityCreation);
			});

			m_onEntityDeletedSlot.Connect(syncSystem.OnEntityDeleted, [this](NetworkSyncSystem*, const NetworkSyncSystem::EntityDestruction& entityDestruction)
			{
				HandleEntityDestruction(entityDestruction);
			});

			m_onEntityInvalidated.Connect(syncSystem.OnEntityInvalidated, [this](NetworkSyncSystem*, const NetworkSyncSystem::EntityMovement& entityMovement)
			{
				if (!m_visibleEntities.UnboundedTest(entityMovement.entityId))
					return;

				m_staticMovementUpdateEvents[entityMovement.entityId] = entityMovement;
			});

			m_onEntityPlayAnimation.Connect(syncSystem.OnEntityPlayAnimation, [this](NetworkSyncSystem*, const NetworkSyncSystem::EntityPlayAnimation& entityPlayAnimation)
			{
				if (!m_visibleEntities.UnboundedTest(entityPlayAnimation.entityId))
					return;

				m_playAnimationEvents[entityPlayAnimation.entityId] = entityPlayAnimation;
			});

			m_onEntitiesHealthUpdate.Connect(syncSystem.OnEntitiesHealthUpdate, [this](NetworkSyncSystem*, const NetworkSyncSystem::EntityHealth* events, std::size_t entityCount)
			{
				for (std::size_t i = 0; i < entityCount; ++i)
				{
					if (!m_visibleEntities.UnboundedTest(events[i].entityId))
						return;

					m_healthUpdateEvents[events[i].entityId] = events[i];
				}
			});

			m_onEntitiesInputUpdate.Connect(syncSystem.OnEntitiesInputUpdate, [this](NetworkSyncSystem*, const NetworkSyncSystem::EntityInputs* events, std::size_t entityCount)
			{
				for (std::size_t i = 0; i < entityCount; ++i)
				{
					if (!m_visibleEntities.UnboundedTest(events[i].entityId))
						return;

					m_inputUpdateEvents[events[i].entityId] = events[i];
				}
			});

			syncSystem.CreateEntities([&](const NetworkSyncSystem::EntityCreation* entitiesCreation, std::size_t entityCount)
			{
				for (std::size_t i = 0; i < entityCount; ++i)
					HandleEntityCreation(entitiesCreation[i]);
			});
		}
	}

	void MatchClientVisibility::HandleEntityCreation(const NetworkSyncSystem::EntityCreation& eventData)
	{
		m_creationEvents[eventData.entityId] = eventData;

		m_visibleEntities.UnboundedSet(eventData.entityId);
	}

	void MatchClientVisibility::HandleEntityDestruction(const NetworkSyncSystem::EntityDestruction& eventData)
	{
		// Only send entity destruction packet if this entity was already created client-side
		auto it = m_creationEvents.find(eventData.entityId);
		if (it != m_creationEvents.end())
			m_creationEvents.erase(it);
		else
			m_destructionEvents.insert(eventData.entityId);

		m_inputUpdateEvents.erase(eventData.entityId);
		m_healthUpdateEvents.erase(eventData.entityId);
		m_playAnimationEvents.erase(eventData.entityId);
		m_staticMovementUpdateEvents.erase(eventData.entityId);

		m_visibleEntities.UnboundedReset(eventData.entityId);
	}

	void MatchClientVisibility::SendMatchState(float elapsedTime)
	{
		Terrain& terrain = m_match.GetTerrain();
		TerrainLayer& layer = terrain.GetLayer(m_activeLayer);
		const NetworkSyncSystem& syncSystem = layer.GetWorld().GetSystem<NetworkSyncSystem>();

		m_matchStatePacket.entities.clear();

		for (auto&& pair : m_staticMovementUpdateEvents)
			BuildMovementPacket(m_matchStatePacket.entities.emplace_back(), pair.second);

		m_staticMovementUpdateEvents.clear();
		syncSystem.MoveEntities([&](const NetworkSyncSystem::EntityMovement* entitiesMovement, std::size_t entityCount)
		{
			for (std::size_t i = 0; i < entityCount; ++i)
				BuildMovementPacket(m_matchStatePacket.entities.emplace_back(), entitiesMovement[i]);
		});

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

}
