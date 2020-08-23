// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Systems/NetworkSyncSystem.hpp>
#include <NDK/Components.hpp>
#include <CoreLib/Match.hpp>
#include <CoreLib/TerrainLayer.hpp>
#include <CoreLib/Components/HealthComponent.hpp>
#include <CoreLib/Components/MatchComponent.hpp>
#include <CoreLib/Components/NetworkSyncComponent.hpp>
#include <CoreLib/Components/PlayerControlledComponent.hpp>
#include <CoreLib/Components/PlayerMovementComponent.hpp>
#include <CoreLib/Components/ScriptComponent.hpp>
#include <CoreLib/Utils.hpp>

namespace bw
{
	NetworkSyncSystem::NetworkSyncSystem(TerrainLayer& layer) :
	m_layer(layer)
	{
		Requires<NetworkSyncComponent, Ndk::NodeComponent>();
		SetMaximumUpdateRate(30.f);
		SetUpdateOrder(100); //< Execute after every other system
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
		{
			auto& entityPhys = entity->GetComponent<Ndk::PhysicsComponent2D>();
			if (entityPhys.IsSleeping())
				continue;

			BuildEvent(m_movementEvents.emplace_back(), entity);
		}

		callback(m_movementEvents.data(), m_movementEvents.size());
	}

	void NetworkSyncSystem::NotifyPhysicsUpdate(const Ndk::EntityHandle& entity)
	{
		if (m_physicsEntities.Has(entity))
			m_physicsUpdateEntities.Insert(entity);
	}

	void NetworkSyncSystem::NotifyScaleUpdate(const Ndk::EntityHandle& entity)
	{
		m_scaleUpdateEntities.Insert(entity);
	}

	void NetworkSyncSystem::BuildEvent(EntityCreation& creationEvent, Ndk::Entity* entity) const
	{
		const NetworkSyncComponent& syncComponent = entity->GetComponent<NetworkSyncComponent>();

		creationEvent.entityId = entity->GetId();
		creationEvent.entityClass = syncComponent.GetEntityClass();

		auto& entityMatch = entity->GetComponent<MatchComponent>();
		creationEvent.uniqueId = entityMatch.GetUniqueId();

		if (const Ndk::EntityHandle& parent = syncComponent.GetParent())
		{
			assert(parent->GetWorld() == entity->GetWorld());
			creationEvent.parent = parent->GetId();
		}

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

			creationEvent.inputs = entityInputs.GetInputs();
		}

		auto& entityNode = entity->GetComponent<Ndk::NodeComponent>();
		creationEvent.scale = entityNode.GetScale().y; //< x is affected by the "looking right" flag

		if (entity->HasComponent<Ndk::PhysicsComponent2D>())
		{
			auto& entityPhys = entity->GetComponent<Ndk::PhysicsComponent2D>();

			creationEvent.position = entityPhys.GetPosition();
			creationEvent.rotation = entityPhys.GetRotation();

			creationEvent.physicsProperties.emplace();
			creationEvent.physicsProperties->angularVelocity = entityPhys.GetAngularVelocity();
			creationEvent.physicsProperties->isSleeping = entityPhys.IsSleeping();
			creationEvent.physicsProperties->linearVelocity = entityPhys.GetVelocity();
			creationEvent.physicsProperties->mass = entityPhys.GetMass();
			creationEvent.physicsProperties->momentOfInertia = entityPhys.GetMomentOfInertia();
		}
		else
		{
			creationEvent.position = Nz::Vector2f(entityNode.GetPosition(Nz::CoordSys_Local));
			creationEvent.rotation = Nz::DegreeAnglef(AngleFromQuaternion(entityNode.GetRotation(Nz::CoordSys_Local))); //< Erk
		}

		if (entity->HasComponent<PlayerMovementComponent>())
		{
			auto& entityPlayerMovement = entity->GetComponent<PlayerMovementComponent>();

			creationEvent.playerMovement.emplace();
			creationEvent.playerMovement->isFacingRight = entityPlayerMovement.IsFacingRight();
		}

		if (entity->HasComponent<PlayerControlledComponent>())
		{
			if (Player* player = entity->GetComponent<PlayerControlledComponent>().GetOwner())
				creationEvent.name = player->GetName();
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

				switch (it->second.type)
				{
					case PropertyType::Entity:
					{
						const Ndk::EntityHandle& propertyEntity = m_layer.GetMatch().RetrieveEntityByUniqueId(std::get<Nz::Int64>(value));
						if (propertyEntity)
						{
							auto& propertyEntityMatch = propertyEntity->GetComponent<MatchComponent>();
							creationEvent.dependentIds.emplace_back(propertyEntityMatch.GetLayerIndex(), propertyEntity->GetId());
						}
						break;
					}

					default:
						break;
				}
			}
		}

		if (entity->HasComponent<WeaponWielderComponent>())
		{
			auto& entityWeaponWielder = entity->GetComponent<WeaponWielderComponent>();

			if (const Ndk::EntityHandle& activeWeapon = entityWeaponWielder.GetActiveWeapon())
				creationEvent.weapon = activeWeapon->GetId();
		}
	}

	void NetworkSyncSystem::BuildEvent(EntityDeath& deathEvent, Ndk::Entity* entity) const
	{
		deathEvent.entityId = entity->GetId();
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
			movementEvent.physicsProperties->isSleeping = entityPhys.IsSleeping();
			movementEvent.physicsProperties->linearVelocity = entityPhys.GetVelocity();
			movementEvent.physicsProperties->mass = entityPhys.GetMass();
		}
		else
		{
			auto& entityNode = entity->GetComponent<Ndk::NodeComponent>();
			movementEvent.position = Nz::Vector2f(entityNode.GetPosition(Nz::CoordSys_Local));
			movementEvent.rotation = AngleFromQuaternion(entityNode.GetRotation(Nz::CoordSys_Local)); //< Erk
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
			auto& entityHealth = entity->GetComponent<HealthComponent>();

			slots.onDied.Connect(entityHealth.OnDied, [&](const HealthComponent* health, const Ndk::EntityHandle&)
			{
				EntityDeath deathEvent;
				BuildEvent(deathEvent, health->GetEntity());

				OnEntityDeath(this, deathEvent);
			});

			slots.onHealthChange.Connect(entityHealth.OnHealthChange, [&](HealthComponent* health)
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

		if (entity->HasComponent<WeaponWielderComponent>())
		{
			auto& entityWeaponWielder = entity->GetComponent<WeaponWielderComponent>();

			slots.onNewWeaponSelection.Connect(entityWeaponWielder.OnNewWeaponSelection, [&](WeaponWielderComponent* wielder, std::size_t /*newWeaponIndex*/)
			{
				m_weaponUpdateEntities.Insert(wielder->GetEntity());
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
		m_physicsUpdateEntities.Remove(entity);
		m_staticEntities.Remove(entity);
		m_weaponUpdateEntities.Remove(entity);

		auto it = m_entitySlots.find(entity->GetId());
		assert(it != m_entitySlots.end());
		m_entitySlots.erase(it);
	}

	void NetworkSyncSystem::OnUpdate(float /*elapsedTime*/)
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
				inputEvent.inputs = entity->GetComponent<InputComponent>().GetInputs();
			}

			m_inputUpdateEntities.Clear();

			OnEntitiesInputUpdate(this, m_inputEvents.data(), m_inputEvents.size());
		}

		if (!m_physicsUpdateEntities.empty())
		{
			m_physicsEvent.clear();

			for (const auto& entity : m_physicsUpdateEntities)
			{
				EntityPhysics& physicsEvent = m_physicsEvent.emplace_back();
				physicsEvent.entityId = entity->GetId();

				auto& entityPhysics = entity->GetComponent<Ndk::PhysicsComponent2D>();
				physicsEvent.isAsleep = entityPhysics.IsSleeping();
				physicsEvent.mass = entityPhysics.GetMass();
				physicsEvent.momentOfInertia = entityPhysics.GetMomentOfInertia();

				if (entity->HasComponent<PlayerMovementComponent>())
				{
					auto& playerMovementData = physicsEvent.playerMovement.emplace();
				
					auto& entityPlayerMovement = entity->GetComponent<PlayerMovementComponent>();
					playerMovementData.jumpHeight = entityPlayerMovement.GetJumpHeight();
					playerMovementData.jumpHeightBoost = entityPlayerMovement.GetJumpBoostHeight();
					playerMovementData.movementSpeed = entityPlayerMovement.GetMovementSpeed();
				}
			}

			m_physicsUpdateEntities.Clear();

			OnEntitiesPhysicsUpdate(this, m_physicsEvent.data(), m_physicsEvent.size());
		}

		if (!m_scaleUpdateEntities.empty())
		{
			m_scaleEvent.clear();

			for (const auto& entity : m_scaleUpdateEntities)
			{
				auto& entityNode = entity->GetComponent<Ndk::NodeComponent>();

				EntityScale& scaleEvent = m_scaleEvent.emplace_back();
				scaleEvent.entityId = entity->GetId();
				scaleEvent.newScale = entityNode.GetScale().y;
			}

			m_scaleUpdateEntities.Clear();

			OnEntitiesScaleUpdate(this, m_scaleEvent.data(), m_scaleEvent.size());
		}

		if (!m_weaponUpdateEntities.empty())
		{
			m_weaponEvents.clear();

			for (const auto& entity : m_weaponUpdateEntities)
			{
				auto& weaponWielder = entity->GetComponent<WeaponWielderComponent>();

				std::size_t selectedWeapon = weaponWielder.GetSelectedWeapon();

				EntityWeapon& weaponEvent = m_weaponEvents.emplace_back();
				weaponEvent.entityId = entity->GetId();

				if (selectedWeapon != WeaponWielderComponent::NoWeapon)
					weaponEvent.weaponId = weaponWielder.GetWeapon(selectedWeapon)->GetId();
			}

			m_weaponUpdateEntities.Clear();

			OnEntitiesWeaponUpdate(this, m_weaponEvents.data(), m_weaponEvents.size());
		}
	}

	Ndk::SystemIndex NetworkSyncSystem::systemIndex;
}
