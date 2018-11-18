// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/Systems/NetworkSyncSystem.hpp>
#include <NDK/Components.hpp>
#include <Shared/Components/HealthComponent.hpp>
#include <Shared/Components/NetworkSyncComponent.hpp>
#include <Shared/Components/PlayerMovementComponent.hpp>
#include <iostream>

namespace bw
{
	NetworkSyncSystem::NetworkSyncSystem()
	{
		Requires<NetworkSyncComponent, Ndk::NodeComponent>();
		SetMaximumUpdateRate(20.f);
	}

	void NetworkSyncSystem::CreateEntities(const std::function<void(const EntityCreation* entityCreation, std::size_t entityCount)>& callback) const
	{
		m_creationEvents.clear();

		for (const Ndk::EntityHandle& entity : GetEntities())
		{
			EntityCreation& creationEvent = m_creationEvents.emplace_back();
			CreateEntity(creationEvent, entity);
		}

		callback(m_creationEvents.data(), m_creationEvents.size());
	}

	void NetworkSyncSystem::DeleteEntities(const std::function<void(const EntityDestruction* entityDestruction, std::size_t entityCount)>& callback) const
	{
		m_destructionEvents.clear();

		for (const Ndk::EntityHandle& entity : GetEntities())
		{
			EntityDestruction& destructionEvent = m_destructionEvents.emplace_back();
			DeleteEntity(destructionEvent, entity);
		}

		callback(m_destructionEvents.data(), m_destructionEvents.size());
	}

	void NetworkSyncSystem::MoveEntities(const std::function<void(const EntityMovement* entityMovement, std::size_t entityCount)>& callback) const
	{
		m_movementEvents.clear();

		for (const Ndk::EntityHandle& entity : m_physicsEntities)
		{
			EntityMovement& movementEvent = m_movementEvents.emplace_back();
			movementEvent.id = entity->GetId();

			if (entity->HasComponent<Ndk::PhysicsComponent2D>())
			{
				//TODO: Handle parents?
				auto& entityPhys = entity->GetComponent<Ndk::PhysicsComponent2D>();
				movementEvent.position = entityPhys.GetPosition();
				movementEvent.rotation = entityPhys.GetRotation();

				movementEvent.physicsProperties.emplace();
				movementEvent.physicsProperties->angularVelocity = entityPhys.GetAngularVelocity();
				movementEvent.physicsProperties->linearVelocity = entityPhys.GetVelocity();
			}
			else
			{
				auto& entityNode = entity->GetComponent<Ndk::NodeComponent>();
				movementEvent.position = Nz::Vector2f(entityNode.GetPosition(Nz::CoordSys_Local));
				movementEvent.rotation = Nz::DegreeAnglef(entityNode.GetRotation(Nz::CoordSys_Local).ToEulerAngles().roll); //< Erk
			}

			if (entity->HasComponent<PlayerMovementComponent>())
			{
				auto& entityPlayerMovement = entity->GetComponent<PlayerMovementComponent>();

				movementEvent.playerMovement.emplace();
				movementEvent.playerMovement->isAirControlling = entityPlayerMovement.IsAirControlling();
				movementEvent.playerMovement->isFacingRight = entityPlayerMovement.IsFacingRight();
			}
		}

		callback(m_movementEvents.data(), m_movementEvents.size());
	}

	void NetworkSyncSystem::CreateEntity(EntityCreation& creationEvent, Ndk::Entity* entity) const
	{
		const NetworkSyncComponent& syncComponent = entity->GetComponent<NetworkSyncComponent>();

		creationEvent.id = entity->GetId();
		creationEvent.entityClass = syncComponent.GetEntityClass();

		if (const Ndk::EntityHandle& parent = syncComponent.GetParent())
			creationEvent.parent = parent->GetId();

		if (entity->HasComponent<HealthComponent>())
		{
			auto& entityHealth = entity->GetComponent<HealthComponent>();

			creationEvent.healthProperties.emplace();
			creationEvent.healthProperties->currentHealth = entityHealth.GetHealth();
			creationEvent.healthProperties->maxHealth = entityHealth.GetMaxHealth();
		}

		if (entity->HasComponent<Ndk::PhysicsComponent2D>())
		{
			auto& entityPhys = entity->GetComponent<Ndk::PhysicsComponent2D>();

			creationEvent.position = entityPhys.GetPosition();
			creationEvent.rotation = entityPhys.GetRotation();

			creationEvent.physicsProperties.emplace();
			creationEvent.physicsProperties->angularVelocity = entityPhys.GetAngularVelocity();
			creationEvent.physicsProperties->linearVelocity = entityPhys.GetVelocity();
		}
		else
		{
			auto& entityNode = entity->GetComponent<Ndk::NodeComponent>();

			creationEvent.position = Nz::Vector2f(entityNode.GetPosition(Nz::CoordSys_Local));
			creationEvent.rotation = Nz::DegreeAnglef(entityNode.GetRotation(Nz::CoordSys_Local).ToEulerAngles().roll); //< Erk
		}

		if (entity->HasComponent<PlayerMovementComponent>())
		{
			auto& entityPlayerMovement = entity->GetComponent<PlayerMovementComponent>();

			creationEvent.playerMovement.emplace();
			creationEvent.playerMovement->isAirControlling = entityPlayerMovement.IsAirControlling();
			creationEvent.playerMovement->isFacingRight = entityPlayerMovement.IsFacingRight();
		}
	}

	void NetworkSyncSystem::DeleteEntity(EntityDestruction& deleteEvent, Ndk::Entity* entity) const
	{
		deleteEvent.id = entity->GetId();
	}

	void NetworkSyncSystem::OnEntityAdded(Ndk::Entity* entity)
	{
		EntityCreation creationEvent;
		CreateEntity(creationEvent, entity);

		OnEntityCreated(this, creationEvent);

		if (entity->HasComponent<Ndk::PhysicsComponent2D>())
			m_physicsEntities.Insert(entity);
		else
			m_staticEntities.Insert(entity);


		assert(m_entitySlots.find(entity->GetId()) == m_entitySlots.end());
		auto& slots = m_entitySlots.emplace(entity->GetId(), EntitySlots()).first.value();

		if (entity->HasComponent<HealthComponent>())
		{
			slots.onHealthChange.Connect(entity->GetComponent<HealthComponent>().OnHealthChange, [&](HealthComponent* health)
			{
				m_healthUpdateEntities.Insert(health->GetEntity());
			});
		}
	}

	void NetworkSyncSystem::OnEntityRemoved(Ndk::Entity* entity)
	{
		EntityDestruction destructionEvent;
		DeleteEntity(destructionEvent, entity);

		OnEntityDeleted(this, destructionEvent);

		m_physicsEntities.Remove(entity);
		m_staticEntities.Remove(entity);

		auto it = m_entitySlots.find(entity->GetId());
		assert(it != m_entitySlots.end());
		m_entitySlots.erase(it);
	}

	void NetworkSyncSystem::OnUpdate(float elapsedTime)
	{
		if (!m_healthUpdateEntities.empty())
		{
			m_healthEvents.clear();

			for (const auto& entity : m_healthUpdateEntities)
			{
				EntityHealth& healthEvent = m_healthEvents.emplace_back();
				healthEvent.id = entity->GetId();
				healthEvent.currentHealth = entity->GetComponent<HealthComponent>().GetHealth();
			}

			m_healthUpdateEntities.Clear();

			OnEntitiesHealthUpdate(this, m_healthEvents.data(), m_healthEvents.size());
		}
	}

	Ndk::SystemIndex NetworkSyncSystem::systemIndex;
}
