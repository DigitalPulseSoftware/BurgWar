// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Systems/NetworkSyncSystem.hpp>
#include <NDK/Components.hpp>
#include <CoreLib/Components/HealthComponent.hpp>
#include <CoreLib/Components/NetworkSyncComponent.hpp>
#include <CoreLib/Components/PlayerMovementComponent.hpp>
#include <CoreLib/Components/ScriptComponent.hpp>
#include <iostream>

namespace bw
{
	NetworkSyncSystem::NetworkSyncSystem()
	{
		Requires<NetworkSyncComponent, Ndk::NodeComponent>();
		SetMaximumUpdateRate(30.f);
	}

	void NetworkSyncSystem::CreateEntities(const std::function<void(const EntityCreation* entityCreation, std::size_t entityCount)>& callback) const
	{
		m_creationEvents.clear();

		for (const Ndk::EntityHandle& entity : GetEntities())
		{
			EntityCreation& creationEvent = m_creationEvents.emplace_back();
			BuildEvent(creationEvent, entity);
		}

		callback(m_creationEvents.data(), m_creationEvents.size());
	}

	void NetworkSyncSystem::DeleteEntities(const std::function<void(const EntityDestruction* entityDestruction, std::size_t entityCount)>& callback) const
	{
		m_destructionEvents.clear();

		for (const Ndk::EntityHandle& entity : GetEntities())
		{
			EntityDestruction& destructionEvent = m_destructionEvents.emplace_back();
			BuildEvent(destructionEvent, entity);
		}

		callback(m_destructionEvents.data(), m_destructionEvents.size());
	}

	void NetworkSyncSystem::MoveEntities(const std::function<void(const EntityMovement* entityMovement, std::size_t entityCount)>& callback) const
	{
		m_movementEvents.clear();

		for (const Ndk::EntityHandle& entity : m_physicsEntities)
			BuildEvent(m_movementEvents.emplace_back(), entity);

		callback(m_movementEvents.data(), m_movementEvents.size());
	}

	void NetworkSyncSystem::BuildEvent(EntityCreation& creationEvent, Ndk::Entity* entity) const
	{
		const NetworkSyncComponent& syncComponent = entity->GetComponent<NetworkSyncComponent>();

		creationEvent.entityId = entity->GetId();
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

		if (entity->HasComponent<InputComponent>())
		{
			auto& entityInputs = entity->GetComponent<InputComponent>();

			creationEvent.inputs = entityInputs.GetInputData();
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
			creationEvent.playerMovement->isFacingRight = entityPlayerMovement.IsFacingRight();
		}

		if (entity->HasComponent<ScriptComponent>())
		{
			auto& scriptComponent = entity->GetComponent<ScriptComponent>();

			const auto& element = scriptComponent.GetElement();

			for (const auto& [key, value] : scriptComponent.GetProperties())
			{
				auto it = element->properties.find(key);
				assert(it != element->properties.end());

				if (!it->second.shared)
					continue;

				creationEvent.properties.emplace(key, value);
			}
		}
	}

	void NetworkSyncSystem::BuildEvent(EntityDestruction& deleteEvent, Ndk::Entity* entity) const
	{
		deleteEvent.entityId = entity->GetId();
	}

	void NetworkSyncSystem::BuildEvent(EntityMovement& movementEvent, Ndk::Entity* entity) const
	{
		movementEvent.entityId = entity->GetId();

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
			movementEvent.playerMovement->isFacingRight = entityPlayerMovement.IsFacingRight();
		}
	}

	void NetworkSyncSystem::OnEntityAdded(Ndk::Entity* entity)
	{
		EntityCreation creationEvent;
		BuildEvent(creationEvent, entity);

		OnEntityCreated(this, creationEvent);

		assert(m_entitySlots.find(entity->GetId()) == m_entitySlots.end());
		auto& slots = m_entitySlots.emplace(entity->GetId(), EntitySlots()).first.value();

		if (entity->HasComponent<Ndk::PhysicsComponent2D>())
			m_physicsEntities.Insert(entity);
		else
		{
			m_staticEntities.Insert(entity);
			slots.onInvalidated.Connect(entity->GetComponent<NetworkSyncComponent>().OnInvalidated, [&](NetworkSyncComponent* netSync)
			{
				EntityMovement movementEvent;
				BuildEvent(movementEvent, netSync->GetEntity());

				OnEntityInvalidated(this, movementEvent);
			});
		}

		if (entity->HasComponent<AnimationComponent>())
		{
			slots.onAnimationStart.Connect(entity->GetComponent<AnimationComponent>().OnAnimationStart, [&](AnimationComponent* anim)
			{
				EntityPlayAnimation event;
				event.animId = anim->GetAnimId();
				event.entityId = anim->GetEntity()->GetId();
				event.startTime = anim->GetStartTime();

				OnEntityPlayAnimation(this, event);
			});
		}

		if (entity->HasComponent<HealthComponent>())
		{
			slots.onHealthChange.Connect(entity->GetComponent<HealthComponent>().OnHealthChange, [&](HealthComponent* health)
			{
				m_healthUpdateEntities.Insert(health->GetEntity());
			});
		}

		if (entity->HasComponent<InputComponent>())
		{
			slots.onInputUpdate.Connect(entity->GetComponent<InputComponent>().OnInputUpdate, [&](InputComponent* input)
			{
				m_inputUpdateEntities.Insert(input->GetEntity());
			});
		}
	}

	void NetworkSyncSystem::OnEntityRemoved(Ndk::Entity* entity)
	{
		EntityDestruction destructionEvent;
		BuildEvent(destructionEvent, entity);

		OnEntityDeleted(this, destructionEvent);

		m_healthUpdateEntities.Remove(entity);
		m_inputUpdateEntities.Remove(entity);
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
				healthEvent.entityId = entity->GetId();
				healthEvent.currentHealth = entity->GetComponent<HealthComponent>().GetHealth();
			}

			m_healthUpdateEntities.Clear();

			OnEntitiesHealthUpdate(this, m_healthEvents.data(), m_healthEvents.size());
		}

		if (!m_inputUpdateEntities.empty())
		{
			m_inputEvents.clear();

			for (const auto& entity : m_inputUpdateEntities)
			{
				EntityInputs& inputEvent = m_inputEvents.emplace_back();
				inputEvent.entityId = entity->GetId();
				inputEvent.inputs = entity->GetComponent<InputComponent>().GetInputData();
			}

			m_inputUpdateEntities.Clear();

			OnEntitiesInputUpdate(this, m_inputEvents.data(), m_inputEvents.size());
		}
	}

	Ndk::SystemIndex NetworkSyncSystem::systemIndex;
}
