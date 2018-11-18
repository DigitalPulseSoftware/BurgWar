// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/MatchClientVisibility.hpp>
#include <Nazara/Core/StackArray.hpp>
#include <Nazara/Core/StackVector.hpp>
#include <Shared/Protocol/Packets.hpp>
#include <Shared/Match.hpp>
#include <Shared/MatchClientSession.hpp>
#include <Shared/Terrain.hpp>
#include <cassert>
#include <queue>

namespace bw
{
	void MatchClientVisibility::Update(float elapsedTime)
	{
		m_entityMovementSendTimer += elapsedTime;
		if (m_entityMovementSendTimer >= m_entityMovementSendInterval)
		{
			m_entityMovementSendTimer -= m_entityMovementSendInterval;

			if (m_activeLayer != NoLayer)
			{
				Terrain& terrain = m_match.GetTerrain();
				TerrainLayer& layer = terrain.GetLayer(m_activeLayer);
				const NetworkSyncSystem& syncSystem = layer.GetWorld().GetSystem<NetworkSyncSystem>();

				Packets::MatchState movementPacket;

				syncSystem.MoveEntities([&](const NetworkSyncSystem::EntityMovement* entitiesMovement, std::size_t entityCount)
				{
					for (std::size_t i = 0; i < entityCount; ++i)
					{
						const auto& eventData = entitiesMovement[i];

						auto& entityData = movementPacket.entities.emplace_back();
						entityData.id = eventData.id;
						entityData.position = eventData.position;
						entityData.rotation = eventData.rotation;

						if (eventData.playerMovement.has_value())
						{
							entityData.playerMovement.emplace();
							entityData.playerMovement->isAirControlling = eventData.playerMovement->isAirControlling;
							entityData.playerMovement->isFacingRight = eventData.playerMovement->isFacingRight;
						}

						if (eventData.physicsProperties.has_value())
						{
							entityData.physicsProperties.emplace();
							entityData.physicsProperties->angularVelocity = eventData.physicsProperties->angularVelocity;
							entityData.physicsProperties->linearVelocity = eventData.physicsProperties->linearVelocity;
						}
					}
				});

				if (!movementPacket.entities.empty())
					m_session.SendPacket(movementPacket);
			}
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

			Packets::DeleteEntities deletePacket;
			syncSystem.DeleteEntities([&](const NetworkSyncSystem::EntityDestruction* entitiesDestruction, std::size_t entityCount)
			{
				deletePacket.entities.reserve(entityCount);
				for (std::size_t i = 0; i < entityCount; ++i)
					HandleEntityDestruction(deletePacket, entitiesDestruction[i]);
			});

			if (!deletePacket.entities.empty())
				m_session.SendPacket(deletePacket);

			m_onEntityCreatedSlot.Disconnect();
			m_onEntityDeletedSlot.Disconnect();
			m_onEntitiesHealthUpdate.Disconnect();
		}

		m_activeLayer = layerIndex;
		if (m_activeLayer != NoLayer)
		{
			/* Create all newly visible entities */
			TerrainLayer& layer = terrain.GetLayer(m_activeLayer);
			NetworkSyncSystem& syncSystem = layer.GetWorld().GetSystem<NetworkSyncSystem>();

			m_onEntityCreatedSlot.Connect(syncSystem.OnEntityCreated, [this](NetworkSyncSystem*, const NetworkSyncSystem::EntityCreation& entityCreation)
			{
				Packets::CreateEntities createPacket;
				HandleEntityCreation(createPacket, entityCreation);

				m_session.SendPacket(createPacket);
			});

			m_onEntityDeletedSlot.Connect(syncSystem.OnEntityDeleted, [this](NetworkSyncSystem*, const NetworkSyncSystem::EntityDestruction& entityDestruction)
			{
				Packets::DeleteEntities deletePacket;
				HandleEntityDestruction(deletePacket, entityDestruction);

				m_session.SendPacket(deletePacket);
			});

			m_onEntitiesHealthUpdate.Connect(syncSystem.OnEntitiesHealthUpdate, [this](NetworkSyncSystem*, const NetworkSyncSystem::EntityHealth* events, std::size_t entityCount)
			{
				Packets::HealthUpdate healthUpdate;

				healthUpdate.entities.reserve(entityCount);
				for (std::size_t i = 0; i < entityCount; ++i)
					HandleEntityHealthUpdate(healthUpdate, events[i]);

				if (!healthUpdate.entities.empty())
					m_session.SendPacket(healthUpdate);
			});

			// Handle parents
			tsl::hopscotch_map<Nz::UInt32 /*entityId*/, std::size_t /*entityId*/> entities;
			Packets::CreateEntities createPacket;
			syncSystem.CreateEntities([&](const NetworkSyncSystem::EntityCreation* entitiesCreation, std::size_t entityCount)
			{
				createPacket.entities.reserve(entityCount);
				for (std::size_t i = 0; i < entityCount; ++i)
				{
					entities[entitiesCreation[i].id] = createPacket.entities.size();
					HandleEntityCreation(createPacket, entitiesCreation[i]);
				}
			});

			if (createPacket.entities.empty())
				return;

			Packets::CreateEntities sortedCreatePacket;
			sortedCreatePacket.entities.reserve(createPacket.entities.size());

			Nz::StackArray<bool> alreadyInPacket = NazaraStackArrayNoInit(bool, createPacket.entities.size());
			alreadyInPacket.fill(false);

			std::function<void(std::size_t entityIndex)> PushEntity;
			PushEntity = [&](std::size_t entityIndex)
			{
				if (!alreadyInPacket[entityIndex])
				{
					auto& entityData = createPacket.entities[entityIndex];
					if (entityData.parentId)
					{
						Nz::UInt32 parentId = entityData.parentId.value();
						auto it = entities.find(parentId);
						if (it != entities.end())
							PushEntity(it->second);
					}

					sortedCreatePacket.entities.emplace_back(std::move(entityData));
					alreadyInPacket[entityIndex] = true;
				}
			};

			for (std::size_t i = 0; i < createPacket.entities.size(); ++i)
				PushEntity(i);

			m_session.SendPacket(sortedCreatePacket);
		}
	}

	void MatchClientVisibility::HandleEntityCreation(Packets::CreateEntities& createPacket, const NetworkSyncSystem::EntityCreation& eventData)
	{
		const NetworkStringStore& networkStringStore = m_match.GetNetworkStringStore();

		auto& entityData = createPacket.entities.emplace_back();
		entityData.id = eventData.id;
		entityData.entityClass = networkStringStore.CheckStringIndex(eventData.entityClass);
		entityData.position = eventData.position;
		entityData.rotation = eventData.rotation;

		if (eventData.parent.has_value())
			entityData.parentId = eventData.parent.value();

		if (eventData.healthProperties.has_value())
		{
			entityData.health.emplace();
			entityData.health->currentHealth = eventData.healthProperties->currentHealth;
			entityData.health->maxHealth = eventData.healthProperties->maxHealth;
		}

		if (eventData.playerMovement.has_value())
		{
			entityData.playerMovement.emplace();
			entityData.playerMovement->isAirControlling = eventData.playerMovement->isAirControlling;
			entityData.playerMovement->isFacingRight = eventData.playerMovement->isFacingRight;
		}

		if (eventData.physicsProperties.has_value())
		{
			entityData.physicsProperties.emplace();
			entityData.physicsProperties->angularVelocity = eventData.physicsProperties->angularVelocity;
			entityData.physicsProperties->linearVelocity = eventData.physicsProperties->linearVelocity;
		}

		m_visibleEntities.UnboundedSet(eventData.id);
	}

	void MatchClientVisibility::HandleEntityDestruction(Packets::DeleteEntities& deletePacket, const NetworkSyncSystem::EntityDestruction& eventData)
	{
		auto& entityData = deletePacket.entities.emplace_back();
		entityData.id = eventData.id;

		m_visibleEntities.UnboundedReset(eventData.id);
	}

	void MatchClientVisibility::HandleEntityHealthUpdate(Packets::HealthUpdate& healthPacket, const NetworkSyncSystem::EntityHealth & eventData)
	{
		// If entity is not visible to us, do nothing
		if (!m_visibleEntities.UnboundedTest(eventData.id))
			return;

		auto& entityData = healthPacket.entities.emplace_back();
		entityData.currentHealth = eventData.currentHealth;
		entityData.id = eventData.id;
	}
}
