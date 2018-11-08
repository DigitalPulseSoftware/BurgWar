// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/Systems/NetworkSyncSystem.hpp>
#include <NDK/Components.hpp>
#include <Shared/Components/NetworkSyncComponent.hpp>
#include <Shared/Components/PlayerMovementComponent.hpp>

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
			auto& entityPhys = entity->GetComponent<Ndk::PhysicsComponent2D>();

			EntityMovement& movementEvent = m_movementEvents.emplace_back();
			movementEvent.id = entity->GetId();
			movementEvent.angularVelocity = entityPhys.GetAngularVelocity();
			movementEvent.linearVelocity = entityPhys.GetVelocity();
			movementEvent.position = entityPhys.GetPosition();
			movementEvent.rotation = entityPhys.GetRotation();

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
		creationEvent.id = entity->GetId();
		creationEvent.entityClass = entity->GetComponent<NetworkSyncComponent>().GetEntityClass();

		if (entity->HasComponent<Ndk::PhysicsComponent2D>())
		{
			auto& entityPhys = entity->GetComponent<Ndk::PhysicsComponent2D>();

			creationEvent.angularVelocity = entityPhys.GetAngularVelocity();
			creationEvent.linearVelocity = entityPhys.GetVelocity();
			creationEvent.position = entityPhys.GetPosition();
			creationEvent.rotation = entityPhys.GetRotation();
		}
		else
		{
			auto& entityNode = entity->GetComponent<Ndk::NodeComponent>();

			creationEvent.angularVelocity = 0.f;
			creationEvent.linearVelocity = Nz::Vector2f::Zero();
			creationEvent.position = Nz::Vector2f(entityNode.GetPosition());
			creationEvent.rotation = Nz::DegreeAnglef(entityNode.GetRotation().ToEulerAngles().roll);
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
	}

	void NetworkSyncSystem::OnEntityRemoved(Ndk::Entity* entity)
	{
		EntityDestruction destructionEvent;
		DeleteEntity(destructionEvent, entity);

		OnEntityDeleted(this, destructionEvent);

		m_physicsEntities.Remove(entity);
		m_staticEntities.Remove(entity);
	}

	void NetworkSyncSystem::OnUpdate(float elapsedTime)
	{
	}

	Ndk::SystemIndex NetworkSyncSystem::systemIndex;
}
