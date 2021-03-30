// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/LocalLayer.hpp>
#include <CoreLib/Components/PlayerMovementComponent.hpp>
#include <ClientLib/ClientSession.hpp>
#include <ClientLib/LocalMatch.hpp>
#include <ClientLib/Components/VisualComponent.hpp>
#include <ClientLib/Systems/FrameCallbackSystem.hpp>
#include <ClientLib/Systems/PostFrameCallbackSystem.hpp>
#include <ClientLib/Systems/VisualInterpolationSystem.hpp>
#include <ClientLib/Scripting/ClientEntityStore.hpp>
#include <ClientLib/Scripting/ClientWeaponStore.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <NDK/Components/PhysicsComponent2D.hpp>
#include <NDK/Systems/LifetimeSystem.hpp>

namespace bw
{
	LocalLayer::LocalLayer(LocalMatch& match, LayerIndex layerIndex, const Nz::Color& backgroundColor) :
	SharedLayer(match, layerIndex),
	m_backgroundColor(backgroundColor),
	m_isEnabled(false),
	m_isPredictionEnabled(false)
	{
		Ndk::World& world = GetWorld();
		world.AddSystem<FrameCallbackSystem>();
		world.AddSystem<PostFrameCallbackSystem>();
		world.AddSystem<VisualInterpolationSystem>();

		OnEntityCreated.Connect([this](LocalLayer* layer, LocalLayerEntity& layerEntity)
		{
			OnEntityVisualCreated(layer, layerEntity);
		});

		OnEntityDelete.Connect([this](LocalLayer* layer, LocalLayerEntity& layerEntity)
		{
			OnEntityVisualDelete(layer, layerEntity);
		});
	}
	
	LocalLayer::LocalLayer(LocalLayer&& layer) noexcept :
	SharedLayer(std::move(layer)),
	m_entities(std::move(layer.m_entities)),
	m_serverEntityIds(std::move(layer.m_serverEntityIds)),
	m_backgroundColor(layer.m_backgroundColor),
	m_isEnabled(layer.m_isEnabled),
	m_isPredictionEnabled(layer.m_isPredictionEnabled)
	{
		for (auto it = m_entities.begin(); it != m_entities.end(); ++it)
		{
			EntityData& entity = it.value();
			entity.onDestruction.Connect(entity.layerEntity.GetEntity()->OnEntityDestruction, [this, uniqueId = entity.layerEntity.GetUniqueId()](Ndk::Entity*)
			{
				HandleEntityDestruction(uniqueId);
			});
		}
		
		OnEntityCreated.Connect([this](LocalLayer* layer, LocalLayerEntity& layerEntity)
		{
			OnEntityVisualCreated(layer, layerEntity);
		});

		OnEntityDelete.Connect([this](LocalLayer* layer, LocalLayerEntity& layerEntity)
		{
			OnEntityVisualDelete(layer, layerEntity);
		});
	}

	LocalLayer::~LocalLayer()
	{
		//FIXME: Disconnect destruction signals because some of them want to create entities (exploding mines)
		for (const Ndk::EntityHandle& entity : GetWorld().GetEntities())
			entity->OnEntityDestruction.Clear();

		Enable(false);
	}

	void LocalLayer::Enable(bool enable)
	{
		if (m_isEnabled == enable)
			return;

		m_isEnabled = enable;

		if (enable)
		{
			OnEnabled(this);
		}
		else
		{
			OnDisabled(this);
			m_entities.clear();
			m_serverEntityIds.clear();
			m_sounds.clear();

			// Since we are disabled, refresh won't be called until we are enabled, refresh the world now to kill entities
			GetWorld().Refresh();
		}
	}

	void LocalLayer::ForEachVisualEntity(const std::function<void(LayerVisualEntity& visualEntity)>& func)
	{
		ForEachLayerEntity([&](LocalLayerEntity& entity)
		{
			func(entity);
		});
	}

	LocalMatch& LocalLayer::GetLocalMatch()
	{
		return static_cast<LocalMatch&>(SharedLayer::GetMatch());
	}

	bool LocalLayer::IsEnabled() const
	{
		return m_isEnabled;
	}

	void LocalLayer::FrameUpdate(float elapsedTime)
	{
		Ndk::World& world = GetWorld();
		world.ForEachSystem([](Ndk::BaseSystem& system)
		{
			system.Enable(false);
		});

		world.GetSystem<Ndk::LifetimeSystem>().Enable(true);
		world.GetSystem<FrameCallbackSystem>().Enable(true);

		world.Update(elapsedTime);
	}

	void LocalLayer::PreFrameUpdate(float elapsedTime)
	{
		Ndk::World& world = GetWorld();
		world.ForEachSystem([](Ndk::BaseSystem& system)
		{
			system.Enable(false);
		});

		world.GetSystem<VisualInterpolationSystem>().Enable(true);

		world.Update(elapsedTime);
	}

	void LocalLayer::PostFrameUpdate(float elapsedTime)
	{
		Ndk::World& world = GetWorld();
		world.ForEachSystem([](Ndk::BaseSystem& system)
		{
			system.Enable(false);
		});

		world.GetSystem<PostFrameCallbackSystem>().Enable(true);

		world.Update(elapsedTime);

		// Sound
		for (auto& soundOpt : m_sounds)
		{
			if (soundOpt)
			{
				if (!soundOpt->sound.Update(elapsedTime))
				{
					OnSoundDelete(this, soundOpt->soundIndex, soundOpt->sound);

					m_freeSoundIds.Set(soundOpt->soundIndex);
					soundOpt.reset();
				}
			}
		}
	}

	LocalLayerEntity& LocalLayer::RegisterEntity(LocalLayerEntity layerEntity)
	{
		assert(layerEntity.GetEntity());
		assert(layerEntity.GetEntity()->GetWorld() == &GetWorld());

		EntityId uniqueId = layerEntity.GetUniqueId();

		if (!layerEntity.IsClientside())
		{
			Nz::UInt32 serverId = layerEntity.GetServerId();
			assert(m_serverEntityIds.find(serverId) == m_serverEntityIds.end());
			m_serverEntityIds.emplace(serverId, uniqueId);
		}

		assert(m_entities.find(uniqueId) == m_entities.end());
		auto it = m_entities.emplace(uniqueId, std::move(layerEntity)).first;

		EntityData& entity = it.value();
		entity.onDestruction.Connect(entity.layerEntity.GetEntity()->OnEntityDestruction, [this, uniqueId](Ndk::Entity*)
		{
			HandleEntityDestruction(uniqueId);
		});

		OnEntityCreated(this, entity.layerEntity);

		return entity.layerEntity;
	}

	LocalLayerSound& LocalLayer::RegisterSound(LocalLayerSound layerEntity)
	{
		std::size_t soundIndex = m_freeSoundIds.FindFirst();
		if (soundIndex == m_freeSoundIds.npos)
		{
			soundIndex = m_freeSoundIds.GetSize();
			m_freeSoundIds.Resize(soundIndex + 1, false);
		}
		else
			m_freeSoundIds.Reset(soundIndex);

		auto& soundOpt = m_sounds.emplace_back();
		soundOpt.emplace(std::move(layerEntity));

		soundOpt->soundIndex = soundIndex;

		OnSoundCreated(this, soundOpt->soundIndex, soundOpt->sound);

		return soundOpt->sound;
	}

	void LocalLayer::SyncVisuals()
	{
		ForEachLayerEntity([&](LocalLayerEntity& layerEntity)
		{
			layerEntity.SyncVisuals();
		});
	}

	void LocalLayer::TickUpdate(float elapsedTime)
	{
		Ndk::World& world = GetWorld();
		world.ForEachSystem([](Ndk::BaseSystem& system)
		{
			system.Enable(true);
		});

		world.GetSystem<Ndk::LifetimeSystem>().Enable(false);
		world.GetSystem<FrameCallbackSystem>().Enable(false);
		world.GetSystem<VisualInterpolationSystem>().Enable(false);

		SharedLayer::TickUpdate(elapsedTime);
	}
	
	void LocalLayer::CreateEntity(Nz::UInt32 entityId, const Packets::Helper::EntityData& entityData)
	{
		static std::string entityPrefix = "entity_";
		static std::string weaponPrefix = "weapon_";

		assert(m_isEnabled);

		LocalMatch& localMatch = GetLocalMatch();
		ClientEntityStore& entityStore = localMatch.GetEntityStore();
		ClientWeaponStore& weaponStore = localMatch.GetWeaponStore();
		const NetworkStringStore& networkStringStore = localMatch.GetNetworkStringStore();

		const std::string& entityClass = networkStringStore.GetString(entityData.entityClass);

		PropertyValueMap properties;
		for (const auto& property : entityData.properties)
		{
			const std::string& propertyName = networkStringStore.GetString(property.name);
			properties.emplace(propertyName, property.value);
		}

		const LocalLayerEntity* parent = nullptr;
		if (entityData.parentId)
		{
			auto idIt = m_serverEntityIds.find(entityData.parentId.value());
			assert(idIt != m_serverEntityIds.end());

			auto entityIt = m_entities.find(idIt->second);
			assert(entityIt != m_entities.end());

			parent = &entityIt.value().layerEntity;
		}

		EntityId uniqueId = static_cast<EntityId>(entityData.uniqueId);

		std::optional<LocalLayerEntity> layerEntity;

		float scale = (entityData.scale) ? entityData.scale.value() : 1.f;

		//FIXME: Entity creation failure should instantiate some placeholder entity
		try
		{
			if (entityClass.compare(0, entityPrefix.size(), entityPrefix) == 0)
			{
				// Entity
				if (std::size_t elementIndex = entityStore.GetElementIndex(entityClass); elementIndex != ClientEntityStore::InvalidIndex)
				{
					auto entity = entityStore.InstantiateEntity(*this, elementIndex, entityId, uniqueId, entityData.position, entityData.rotation, scale, properties, (parent) ? parent->GetEntity() : Ndk::EntityHandle::InvalidHandle);
					if (!entity)
					{
						bwLog(GetMatch().GetLogger(), LogLevel::Error, "Failed to instantiate entity {0} of type {1}", uniqueId, entityClass);
						return;
					}

					layerEntity.emplace(std::move(entity.value()));
				}
				else
				{
					bwLog(GetMatch().GetLogger(), LogLevel::Error, "Failed to instantiate entity {0}: {0}", uniqueId, entityClass);
					return;
				}
			}
			else if (entityClass.compare(0, weaponPrefix.size(), weaponPrefix) == 0)
			{
				// Weapon
				if (std::size_t weaponIndex = weaponStore.GetElementIndex(entityClass); weaponIndex != ClientEntityStore::InvalidIndex)
				{
					if (!parent)
					{
						bwLog(GetMatch().GetLogger(), LogLevel::Error, "Weapon entities should always have parents", entityClass);
						return;
					}

					auto weapon = weaponStore.InstantiateWeapon(*this, weaponIndex, entityId, uniqueId, properties, parent->GetEntity());
					if (!weapon)
					{
						bwLog(GetMatch().GetLogger(), LogLevel::Error, "Failed to instantiate weapon {0} of type {1}", uniqueId, entityClass);
						return;
					}

					weapon->Disable(); //< Disable weapon entities by default

					layerEntity.emplace(std::move(weapon.value()));
				}
				else
				{
					bwLog(GetMatch().GetLogger(), LogLevel::Error, "Failed to instantiate weapon {0}: unknown entity type: {0}", uniqueId, entityClass);
					return;
				}
			}
			else
			{
				// Unknown
				bwLog(GetMatch().GetLogger(), LogLevel::Error, "Failed to decode element {0} entity type: {1}", uniqueId, entityClass);
				return;
			}
		}
		catch (const std::exception & e)
		{
			bwLog(GetMatch().GetLogger(), LogLevel::Error, "Failed to instantiate element {0} of type {1}: {2}", entityId, entityClass, e.what());
			return;
		}

		if (entityData.health)
			layerEntity->InitializeHealth(entityData.health->maxHealth, entityData.health->currentHealth);

		if (entityData.physicsProperties)
		{
			auto& physProperties = *entityData.physicsProperties;

			if (layerEntity->GetEntity()->HasComponent<Ndk::PhysicsComponent2D>())
			{
				auto& entityPhys = layerEntity->GetEntity()->GetComponent<Ndk::PhysicsComponent2D>();
				entityPhys.SetMass(physProperties.mass, false);
				entityPhys.SetMomentOfInertia(physProperties.momentOfInertia);
				entityPhys.SetAngularVelocity(physProperties.angularVelocity);
				entityPhys.SetVelocity(physProperties.linearVelocity);

				if (physProperties.isAsleep)
					entityPhys.ForceSleep();
			}
			else
				bwLog(GetMatch().GetLogger(), LogLevel::Warning, "Entity {0} has physical properties but is not physical client-side");
		}

		RegisterEntity(std::move(layerEntity.value()));
	}

	void LocalLayer::HandleEntityDestruction(EntityId uniqueId)
	{
		auto it = m_entities.find(uniqueId);
		assert(it != m_entities.end());

		EntityData& entity = it.value();
		OnEntityDelete(this, entity.layerEntity);

		if (!entity.layerEntity.IsClientside())
		{
			std::size_t erasedCount = m_serverEntityIds.erase(entity.layerEntity.GetServerId());
			NazaraUnused(erasedCount);
			assert(erasedCount == 1);
		}

		m_entities.erase(it);
	}

	void LocalLayer::HandlePacket(const Packets::CreateEntities::Entity* entities, std::size_t entityCount)
	{
		assert(m_isEnabled);

		for (std::size_t i = 0; i < entityCount; ++i)
		{
			Nz::UInt32 entityId = entities[i].id;
			auto& entityData = entities[i].data;

			CreateEntity(entityId, entityData);
		}
	}

	void LocalLayer::HandlePacket(const Packets::DeleteEntities::Entity* entities, std::size_t entityCount)
	{
		assert(m_isEnabled);

		for (std::size_t i = 0; i < entityCount; ++i)
		{
			if (EntityId uniqueId = GetUniqueIdByServerId(entities[i].id); uniqueId != 0)
				HandleEntityDestruction(uniqueId);
		}
	}

	void LocalLayer::HandlePacket(const Packets::EnableLayer::Entity* entities, std::size_t entityCount)
	{
		assert(m_isEnabled);

		for (std::size_t i = 0; i < entityCount; ++i)
		{
			Nz::UInt32 entityId = entities[i].id;
			auto& entityData = entities[i].data;

			CreateEntity(entityId, entityData);
		}
	}

	void LocalLayer::HandlePacket(const Packets::EntitiesAnimation::Entity* entities, std::size_t entityCount)
	{
		assert(m_isEnabled);

		for (std::size_t i = 0; i < entityCount; ++i)
		{
			Nz::UInt32 entityId = entities[i].entityId;
			Nz::UInt8 animationId = entities[i].animId;

			auto entityOpt = GetEntityByServerId(entityId);
			if (!entityOpt)
				continue;

			LocalLayerEntity& localEntity = entityOpt.value();
			localEntity.UpdateAnimation(animationId);
		}
	}

	void LocalLayer::HandlePacket(const Packets::EntitiesDeath::Entity* entities, std::size_t entityCount)
	{
		assert(m_isEnabled);

		for (std::size_t i = 0; i < entityCount; ++i)
		{
			Nz::UInt32 entityId = entities[i].id;
			if (EntityId uniqueId = GetUniqueIdByServerId(entityId); uniqueId != 0)
			{
				auto entityOpt = GetEntity(uniqueId);
				assert(entityOpt);

				LocalLayerEntity& localEntity = entityOpt.value();
				if (localEntity.HasHealth())
					localEntity.UpdateHealth(0);
				else
					bwLog(GetMatch().GetLogger(), LogLevel::Error, "Received death event for entity {} which has no life", localEntity.GetUniqueId());

				HandleEntityDestruction(uniqueId);
			}
		}
	}

	void LocalLayer::HandlePacket(const Packets::EntitiesInputs::Entity* entities, std::size_t entityCount)
	{
		assert(m_isEnabled);

		for (std::size_t i = 0; i < entityCount; ++i)
		{
			Nz::UInt32 entityId = entities[i].id;
			const auto& inputs = entities[i].inputs;

			auto entityOpt = GetEntityByServerId(entityId);
			if (!entityOpt)
				continue;

			LocalLayerEntity& localEntity = entityOpt.value();
			localEntity.UpdateInputs(inputs);
		}
	}

	void LocalLayer::HandlePacket(const Packets::EntitiesScale::Entity* entities, std::size_t entityCount)
	{
		assert(m_isEnabled);

		for (std::size_t i = 0; i < entityCount; ++i)
		{
			Nz::UInt32 entityId = entities[i].id;
			float newScale = entities[i].newScale;

			auto entityOpt = GetEntityByServerId(entityId);
			if (!entityOpt)
				continue;

			LocalLayerEntity& localEntity = entityOpt.value();
			localEntity.UpdateScale(newScale);
		}
	}

	void LocalLayer::HandlePacket(const Packets::EntityPhysics& packet)
	{
		assert(packet.entityId.layerId == GetLayerIndex());

		auto entityOpt = GetEntityByServerId(packet.entityId.entityId);
		if (!entityOpt)
			return;

		LocalLayerEntity& localEntity = *entityOpt;
		if (localEntity.IsPhysical())
		{
			const Ndk::EntityHandle& entity = localEntity.GetEntity();

			auto& entityPhys = entity->GetComponent<Ndk::PhysicsComponent2D>();
			entityPhys.SetMass(packet.mass, false);
			entityPhys.SetMomentOfInertia(packet.momentOfInertia);

			if (packet.asleep)
				entityPhys.ForceSleep();

			if (packet.playerMovement)
			{
				auto& packetPlayerMovement = packet.playerMovement.value();

				if (entity->HasComponent<PlayerMovementComponent>())
				{
					auto& entityPlayerMovement = entity->GetComponent<PlayerMovementComponent>();
					entityPlayerMovement.UpdateJumpBoostHeight(packetPlayerMovement.jumpHeightBoost);
					entityPlayerMovement.UpdateJumpHeight(packetPlayerMovement.jumpHeight);
					entityPlayerMovement.UpdateMovementSpeed(packetPlayerMovement.movementSpeed);
				}
			}
		}
	}

	void LocalLayer::HandlePacket(const Packets::EntityWeapon& packet)
	{
		assert(packet.entityId.layerId == GetLayerIndex());

		auto entityOpt = GetEntityByServerId(packet.entityId.entityId);
		if (!entityOpt)
			return;

		LocalLayerEntity& localEntity = *entityOpt;
		if (packet.weaponEntityId != Packets::EntityWeapon::NoWeapon)
		{
			auto newWeaponOpt = GetEntityByServerId(packet.weaponEntityId);
			if (!newWeaponOpt)
				return;

			LocalLayerEntity& newWeapon = newWeaponOpt.value();
			localEntity.UpdateWeaponEntity(newWeapon.CreateHandle<LocalLayerEntity>());
		}
		else
			localEntity.UpdateWeaponEntity({});
	}

	void LocalLayer::HandlePacket(const Packets::HealthUpdate::Entity* entities, std::size_t entityCount)
	{
		assert(m_isEnabled);

		for (std::size_t i = 0; i < entityCount; ++i)
		{
			Nz::UInt32 entityId = entities[i].id;
			Nz::UInt16 currentHealth = entities[i].currentHealth;

			auto entityOpt = GetEntityByServerId(entityId);
			if (!entityOpt)
				continue;

			LocalLayerEntity& localEntity = entityOpt.value();
			if (localEntity.HasHealth())
				localEntity.UpdateHealth(currentHealth);
			else
				bwLog(GetMatch().GetLogger(), LogLevel::Error, "Received health data for entity {} which has none", localEntity.GetUniqueId());
		}
	}
}
