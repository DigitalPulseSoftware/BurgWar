// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Systems/NetworkSyncSystem.hpp>
#include <CoreLib/Match.hpp>
#include <CoreLib/TerrainLayer.hpp>
#include <CoreLib/Utils.hpp>
#include <CoreLib/Components/HealthComponent.hpp>
#include <CoreLib/Components/MatchComponent.hpp>
#include <CoreLib/Components/NetworkSyncComponent.hpp>
#include <CoreLib/Components/OwnerComponent.hpp>
#include <CoreLib/Components/PlayerMovementComponent.hpp>
#include <CoreLib/Components/ScriptComponent.hpp>
#include <Nazara/Core/Components/NodeComponent.hpp>

namespace bw
{
	NetworkSyncSystem::NetworkSyncSystem(entt::registry& registry, TerrainLayer& layer) :
	m_registry(registry),
	m_layer(layer)
	{
		m_freeNetworkIds.Resize(1024, true);

		m_observer.connect(m_registry, entt::collector.group<NetworkSyncComponent, Nz::NodeComponent>());
		//Requires<NetworkSyncComponent, Nz::NodeComponent>();
		//SetMaximumUpdateRate(30.f);
	}

	void NetworkSyncSystem::CreateEntities(const std::function<void(const EntityCreation* entityCreation, std::size_t entityCount)>& callback) const
	{
		m_creationEvents.clear();

		auto view = m_registry.view<NetworkSyncComponent, Nz::NodeComponent>();
		for (entt::entity entity : view)
		{
			EntityCreation& creationEvent = m_creationEvents.emplace_back();
			BuildEvent(creationEvent, entity);
		}

		callback(m_creationEvents.data(), m_creationEvents.size());
	}

	void NetworkSyncSystem::DeleteEntities(const std::function<void(const EntityDestruction* entityDestruction, std::size_t entityCount)>& callback) const
	{
		m_destructionEvents.clear();

		auto view = m_registry.view<NetworkSyncComponent, Nz::NodeComponent>();
		for (entt::entity entity : view)
		{
			EntityDestruction& destructionEvent = m_destructionEvents.emplace_back();
			BuildEvent(destructionEvent, entity);
		}

		callback(m_destructionEvents.data(), m_destructionEvents.size());
	}

	void NetworkSyncSystem::MoveEntities(const std::function<void(const EntityMovement* entityMovement, std::size_t entityCount)>& callback) const
	{
		m_movementEvents.clear();

		for (entt::entity entity : m_physicsEntities)
		{
			auto& entityPhys = m_registry.get<Nz::RigidBody2DComponent>(entity);
			if (entityPhys.IsSleeping())
				continue;

			BuildEvent(m_movementEvents.emplace_back(), entity);
		}

		callback(m_movementEvents.data(), m_movementEvents.size());
	}

	Nz::UInt32 NetworkSyncSystem::AllocateNetworkId()
	{
		std::size_t freeId = m_freeNetworkIds.FindFirst();
		if (freeId == m_freeNetworkIds.npos)
		{
			freeId = m_freeNetworkIds.GetSize();
			m_freeNetworkIds.Resize(m_freeNetworkIds.GetSize() * 2, true);
		}

		m_freeNetworkIds.Set(freeId, false);
		return Nz::SafeCast<Nz::UInt32>(freeId);
	}

	void NetworkSyncSystem::BuildEvent(EntityCreation& creationEvent, entt::entity entity) const
	{
		const NetworkSyncComponent& syncComponent = m_registry.get<NetworkSyncComponent>(entity);

		creationEvent.entityId = syncComponent.GetNetworkId();
		creationEvent.entityClass = syncComponent.GetEntityClass();

		auto& entityMatch = m_registry.get<MatchComponent>(entity);
		creationEvent.uniqueId = entityMatch.GetUniqueId();

		if (entt::handle parent = syncComponent.GetParent())
		{
			assert(parent.registry() == &m_registry);
			creationEvent.parent = parent.get<NetworkSyncComponent>().GetNetworkId();
		}

		if (HealthComponent* healthComponent = m_registry.try_get<HealthComponent>(entity))
		{
			creationEvent.healthProperties.emplace();
			creationEvent.healthProperties->currentHealth = healthComponent->GetHealth();
			creationEvent.healthProperties->maxHealth = healthComponent->GetMaxHealth();
		}

		if (InputComponent* entityInputs = m_registry.try_get<InputComponent>(entity))
			creationEvent.inputs = entityInputs->GetInputs();

		if (OwnerComponent* entityOwner = m_registry.try_get<OwnerComponent>(entity))
			creationEvent.playerOwner = entityOwner->GetOwner();
		else
			creationEvent.playerOwner = nullptr;

		auto& entityNode = m_registry.get<Nz::NodeComponent>(entity);
		creationEvent.scale = entityNode.GetScale().y; //< x is affected by the "looking right" flag

		if (Nz::RigidBody2DComponent* entityPhys = m_registry.try_get<Nz::RigidBody2DComponent>(entity))
		{
			creationEvent.position = entityPhys->GetPosition();
			creationEvent.rotation = entityPhys->GetRotation();

			creationEvent.physicsProperties.emplace();
			creationEvent.physicsProperties->angularVelocity = entityPhys->GetAngularVelocity();
			creationEvent.physicsProperties->isSleeping = entityPhys->IsSleeping();
			creationEvent.physicsProperties->linearVelocity = entityPhys->GetVelocity();
			creationEvent.physicsProperties->mass = entityPhys->GetMass();
			creationEvent.physicsProperties->momentOfInertia = entityPhys->GetMomentOfInertia();
		}
		else
		{
			creationEvent.position = Nz::Vector2f(entityNode.GetPosition());
			creationEvent.rotation = Nz::DegreeAnglef(AngleFromQuaternion(entityNode.GetRotation())); //< Eww
		}

		if (PlayerMovementComponent* entityPlayerMovement = m_registry.try_get<PlayerMovementComponent>(entity))
		{
			creationEvent.playerMovement.emplace();
			creationEvent.playerMovement->isFacingRight = entityPlayerMovement->IsFacingRight();
		}

		if (ScriptComponent* scriptComponent = m_registry.try_get<ScriptComponent>(entity))
		{
			const auto& element = scriptComponent->GetElement();

			for (const auto& [key, value] : scriptComponent->GetProperties())
			{
				auto it = element->properties.find(key);
				assert(it != element->properties.end());

				if (!it->second.shared)
					continue;

				creationEvent.properties.emplace(key, value);

				auto RegisterDependentId = [&](EntityId entityId)
				{
					entt::handle propertyEntity = m_layer.GetMatch().RetrieveEntityByUniqueId(entityId);
					if (propertyEntity)
					{
						Nz::UInt32 networkId = propertyEntity.get<NetworkSyncComponent>().GetNetworkId();

						auto& propertyEntityMatch = propertyEntity.get<MatchComponent>();
						creationEvent.dependentIds.emplace_back(propertyEntityMatch.GetLayerIndex(), networkId);
					}
				};

				std::visit([&](auto&& propertyValue)
				{
					using T = std::decay_t<decltype(propertyValue)>;
					using TypeExtractor = PropertyTypeExtractor<T>;
					constexpr bool IsArray = TypeExtractor::IsArray;

					if constexpr (TypeExtractor::Property == PropertyType::Entity)
					{
						if constexpr (IsArray)
						{
							for (auto& id : propertyValue)
								RegisterDependentId(id);
						}
						else
							RegisterDependentId(propertyValue.value);

					}
				}, value);
			}
		}

		if (WeaponWielderComponent* entityWeaponWielder = m_registry.try_get<WeaponWielderComponent>(entity))
		{
			if (entt::handle activeWeapon = entityWeaponWielder->GetActiveWeapon())
			{
				Nz::UInt32 networkId = activeWeapon.get<NetworkSyncComponent>().GetNetworkId();
				creationEvent.weapon = networkId;
			}
		}
	}

	void NetworkSyncSystem::BuildEvent(EntityDeath& deathEvent, entt::entity entity) const
	{
		const NetworkSyncComponent& syncComponent = m_registry.get<NetworkSyncComponent>(entity);
		deathEvent.entityId = syncComponent.GetNetworkId();
	}

	void NetworkSyncSystem::BuildEvent(EntityDestruction& deleteEvent, entt::entity entity) const
	{
		const NetworkSyncComponent& syncComponent = m_registry.get<NetworkSyncComponent>(entity);
		deleteEvent.entityId = syncComponent.GetNetworkId();
	}

	void NetworkSyncSystem::BuildEvent(EntityMovement& movementEvent, entt::entity entity) const
	{
		const NetworkSyncComponent& syncComponent = m_registry.get<NetworkSyncComponent>(entity);
		movementEvent.entityId = syncComponent.GetNetworkId();

		if (Nz::RigidBody2DComponent* entityPhys = m_registry.try_get<Nz::RigidBody2DComponent>(entity))
		{
			//TODO: Handle parents?
			movementEvent.position = entityPhys->GetPosition();
			movementEvent.rotation = entityPhys->GetRotation();

			movementEvent.physicsProperties.emplace();
			movementEvent.physicsProperties->angularVelocity = entityPhys->GetAngularVelocity();
			movementEvent.physicsProperties->isSleeping = entityPhys->IsSleeping();
			movementEvent.physicsProperties->linearVelocity = entityPhys->GetVelocity();
			movementEvent.physicsProperties->mass = entityPhys->GetMass();
		}
		else
		{
			auto& entityNode = m_registry.get<Nz::NodeComponent>(entity);
			movementEvent.position = Nz::Vector2f(entityNode.GetPosition());
			movementEvent.rotation = AngleFromQuaternion(entityNode.GetRotation()); //< Ew
		}

		if (PlayerMovementComponent* entityPlayerMovement = m_registry.try_get<PlayerMovementComponent>(entity))
		{
			movementEvent.playerMovement.emplace();
			movementEvent.playerMovement->isFacingRight = entityPlayerMovement->IsFacingRight();
		}
	}

	void NetworkSyncSystem::FreeNetworkId(Nz::UInt32 networkId)
	{
		assert(!m_freeNetworkIds.UnboundedTest(networkId));
		m_freeNetworkIds.Set(networkId, true);
	}

	void NetworkSyncSystem::HandleNewEntity(entt::entity entity)
	{
		EntityCreation creationEvent;
		BuildEvent(creationEvent, entity);

		OnEntityCreated(this, creationEvent);

		assert(m_entitySlots.find(entity) == m_entitySlots.end());
		auto& slots = m_entitySlots.emplace(entity, EntitySlots()).first.value();

		if (m_registry.try_get<Nz::RigidBody2DComponent>(entity))
			m_physicsEntities.insert(entity);
		else
			m_staticEntities.insert(entity);

		Nz::UInt32 networkId = creationEvent.entityId;

		if (AnimationComponent* entityAnim = m_registry.try_get<AnimationComponent>(entity))
		{
			slots.onAnimationStart.Connect(entityAnim->OnAnimationStart, [this, networkId](AnimationComponent* anim)
			{
				EntityPlayAnimation event;
				event.animId = anim->GetAnimId();
				event.entityId = networkId;
				event.startTime = anim->GetStartTime().AsMicroseconds();

				OnEntityPlayAnimation(this, event);
			});
		}

		if (HealthComponent* entityHealth = m_registry.try_get<HealthComponent>(entity))
		{
			slots.onDied.Connect(entityHealth->OnDie, [this](const HealthComponent* health, entt::entity /*attacker*/)
			{
				EntityDeath deathEvent;
				BuildEvent(deathEvent, health->GetEntity());

				OnEntityDeath(this, deathEvent);
			});

			slots.onHealthChange.Connect(entityHealth->OnHealthChange, [this, networkId](HealthComponent* health, Nz::UInt16 /*newHealth*/, entt::entity /*dealer*/)
			{
				m_healthUpdateEntities.insert(health->GetEntity());
			});
		}

		if (InputComponent* entityInput = m_registry.try_get<InputComponent>(entity))
		{
			slots.onInputUpdate.Connect(entityInput->OnInputUpdate, [this](InputComponent* input)
			{
				m_inputUpdateEntities.insert(input->GetEntity());
			});
		}

		if (WeaponWielderComponent* entityWeaponWielder = m_registry.try_get<WeaponWielderComponent>(entity))
		{
			slots.onNewWeaponSelection.Connect(entityWeaponWielder->OnNewWeaponSelection, [this](WeaponWielderComponent* wielder, std::size_t /*newWeaponIndex*/)
			{
				m_weaponUpdateEntities.insert(wielder->GetEntity());
			});
		}
	}

	void NetworkSyncSystem::OnComponentRemoved(entt::entity entity)
	{
		EntityDestruction destructionEvent;
		BuildEvent(destructionEvent, entity);

		OnEntityDeleted(this, destructionEvent);

		m_healthUpdateEntities.erase(entity);
		m_inputUpdateEntities.erase(entity);
		m_physicsEntities.erase(entity);
		m_physicsUpdateEntities.erase(entity);
		m_staticEntities.erase(entity);
		m_weaponUpdateEntities.erase(entity);

		auto it = m_entitySlots.find(entity);
		assert(it != m_entitySlots.end());
		m_entitySlots.erase(it);
	}

	void NetworkSyncSystem::Update(Nz::Time /*elapsedTime*/)
	{
		if (!m_healthUpdateEntities.empty())
		{
			m_healthEvents.clear();

			for (entt::entity entity : m_healthUpdateEntities)
			{
				const auto& entityHealth = m_registry.get<HealthComponent>(entity);
				const auto& entityNetwork = m_registry.get<NetworkSyncComponent>(entity);

				EntityHealth& healthEvent = m_healthEvents.emplace_back();
				healthEvent.entityId = entityNetwork.GetNetworkId();
				healthEvent.currentHealth = entityHealth.GetHealth();
			}
			m_healthUpdateEntities.clear();

			OnEntitiesHealthUpdate(this, m_healthEvents.data(), m_healthEvents.size());
		}

		if (!m_inputUpdateEntities.empty())
		{
			m_inputEvents.clear();

			for (entt::entity entity : m_inputUpdateEntities)
			{
				const auto& entityInputs = m_registry.get<InputComponent>(entity);
				const auto& entityNetwork = m_registry.get<NetworkSyncComponent>(entity);

				EntityInputs& inputEvent = m_inputEvents.emplace_back();
				inputEvent.entityId = entityNetwork.GetNetworkId();
				inputEvent.inputs = entityInputs.GetInputs();
			}

			m_inputUpdateEntities.clear();

			OnEntitiesInputUpdate(this, m_inputEvents.data(), m_inputEvents.size());
		}

		if (!m_movedStaticEntities.empty())
		{
			for (const auto& entity : m_movedStaticEntities)
			{
				EntityMovement movementEvent;
				BuildEvent(movementEvent, entity);

				OnEntityInvalidated(this, movementEvent);
			}

			m_movedStaticEntities.clear();
		}

		if (!m_physicsUpdateEntities.empty())
		{
			m_physicsEvent.clear();

			for (entt::entity entity : m_physicsUpdateEntities)
			{
				const auto& entityNetwork = m_registry.get<NetworkSyncComponent>(entity);
				const auto& entityPhysics = m_registry.get<Nz::RigidBody2DComponent>(entity);

				EntityPhysics& physicsEvent = m_physicsEvent.emplace_back();
				physicsEvent.entityId = entityNetwork.GetNetworkId();

				physicsEvent.isAsleep = entityPhysics.IsSleeping();
				physicsEvent.mass = entityPhysics.GetMass();
				physicsEvent.momentOfInertia = entityPhysics.GetMomentOfInertia();

				if (PlayerMovementComponent* entityPlayerMovement = m_registry.try_get<PlayerMovementComponent>(entity))
				{
					auto& playerMovementData = physicsEvent.playerMovement.emplace();
				
					playerMovementData.jumpHeight = entityPlayerMovement->GetJumpHeight();
					playerMovementData.jumpHeightBoost = entityPlayerMovement->GetJumpBoostHeight();
					playerMovementData.movementSpeed = entityPlayerMovement->GetMovementSpeed();
				}
			}

			m_physicsUpdateEntities.clear();

			OnEntitiesPhysicsUpdate(this, m_physicsEvent.data(), m_physicsEvent.size());
		}

		if (!m_scaleUpdateEntities.empty())
		{
			m_scaleEvent.clear();

			for (entt::entity entity : m_scaleUpdateEntities)
			{
				const auto& entityNetwork = m_registry.get<NetworkSyncComponent>(entity);
				const auto& entityNode = m_registry.get<Nz::NodeComponent>(entity);

				EntityScale& scaleEvent = m_scaleEvent.emplace_back();
				scaleEvent.entityId = entityNetwork.GetNetworkId();
				scaleEvent.newScale = entityNode.GetScale().y;
			}

			m_scaleUpdateEntities.clear();

			OnEntitiesScaleUpdate(this, m_scaleEvent.data(), m_scaleEvent.size());
		}

		if (!m_weaponUpdateEntities.empty())
		{
			m_weaponEvents.clear();

			for (entt::entity entity : m_weaponUpdateEntities)
			{
				const auto& entityNetwork = m_registry.get<NetworkSyncComponent>(entity);
				const auto& entityWeaponWielder = m_registry.get<WeaponWielderComponent>(entity);

				std::size_t selectedWeapon = entityWeaponWielder.GetSelectedWeapon();

				EntityWeapon& weaponEvent = m_weaponEvents.emplace_back();
				weaponEvent.entityId = entityNetwork.GetNetworkId();

				if (selectedWeapon != WeaponWielderComponent::NoWeapon)
					weaponEvent.weaponId = entityWeaponWielder.GetWeapon(selectedWeapon).get<NetworkSyncComponent>().GetNetworkId();
			}

			m_weaponUpdateEntities.clear();

			OnEntitiesWeaponUpdate(this, m_weaponEvents.data(), m_weaponEvents.size());
		}
	}
}
