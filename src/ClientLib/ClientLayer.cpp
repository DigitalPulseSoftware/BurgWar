// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/ClientLayer.hpp>
#include <CoreLib/Components/DestructionWatcherComponent.hpp>
#include <CoreLib/Components/PlayerMovementComponent.hpp>
#include <ClientLib/ClientSession.hpp>
#include <ClientLib/ClientMatch.hpp>
#include <ClientLib/Components/ClientOwnerComponent.hpp>
#include <ClientLib/Components/VisualComponent.hpp>
#include <ClientLib/Systems/FrameCallbackSystem.hpp>
#include <ClientLib/Systems/PostFrameCallbackSystem.hpp>
#include <ClientLib/Systems/VisualInterpolationSystem.hpp>
#include <ClientLib/Scripting/ClientEntityStore.hpp>
#include <ClientLib/Scripting/ClientWeaponStore.hpp>
#include <Nazara/Core/Components/LifetimeComponent.hpp>
#include <Nazara/Physics2D/Components/RigidBody2DComponent.hpp>
#include <Nazara/Utility/Components/NodeComponent.hpp>

namespace bw
{
	ClientLayer::ClientLayer(ClientMatch& match, LayerIndex layerIndex, const Nz::Color& backgroundColor) :
	ClientEditorLayer(match, layerIndex),
	m_backgroundColor(backgroundColor),
	m_isEnabled(false),
	m_isPredictionEnabled(false)
	{
		Nz::SystemGraph& systemGraph = GetSystemGraph();
		systemGraph.AddSystem<FrameCallbackSystem>();
		systemGraph.AddSystem<PostFrameCallbackSystem>();
		systemGraph.AddSystem<VisualInterpolationSystem>();

		OnEntityCreated.Connect([this](ClientLayer* layer, ClientLayerEntity& layerEntity)
		{
			OnEntityVisualCreated(layer, layerEntity);
		});

		OnEntityDelete.Connect([this](ClientLayer* layer, ClientLayerEntity& layerEntity)
		{
			OnEntityVisualDelete(layer, layerEntity);
		});
	}
	
	ClientLayer::~ClientLayer()
	{
		Enable(false);
	}

	void ClientLayer::Enable(bool enable)
	{
		if (m_isEnabled == enable)
			return;

		m_isEnabled = enable;

		if (enable)
			OnEnabled(this);
		else
		{
			OnDisabled(this);
			m_sounds.clear();
			m_freeSoundIds.Clear();

			assert(m_entities.empty());
			assert(m_serverEntityIds.empty());
		}
	}

	void ClientLayer::ForEachVisualEntity(const std::function<void(LayerVisualEntity& visualEntity)>& func)
	{
		ForEachLayerEntity([&](ClientLayerEntity& entity)
		{
			func(entity);
		});
	}

	ClientMatch& ClientLayer::GetClientMatch()
	{
		return static_cast<ClientMatch&>(SharedLayer::GetMatch());
	}

	bool ClientLayer::IsEnabled() const
	{
		return m_isEnabled;
	}

	void ClientLayer::PostFrameUpdate(float elapsedTime)
	{
		ClientEditorLayer::PostFrameUpdate(elapsedTime);

		// Sound
		for (std::size_t i = 0; i < m_sounds.size(); ++i)
		{
			auto& soundOpt = m_sounds[i];
			if (!soundOpt)
				continue;

			if (!soundOpt->sound.Update(elapsedTime))
			{
				OnSoundDelete(this, i, soundOpt->sound);

				m_freeSoundIds.Set(i);
				soundOpt.reset();
			}
		}
	}

	ClientLayerEntity& ClientLayer::RegisterEntity(ClientLayerEntity layerEntity)
	{
		assert(layerEntity.GetEntity());
		assert(layerEntity.GetEntity().registry() == &GetWorld());

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
		entt::handle handle = entity.layerEntity.GetEntity();
		entity.onDestruction.Connect(handle.get_or_emplace<DestructionWatcherComponent>(handle).OnDestruction, [this, uniqueId](DestructionWatcherComponent*)
		{
			HandleEntityDestruction(uniqueId);
		});

		OnEntityCreated(this, entity.layerEntity);

		return entity.layerEntity;
	}

	ClientLayerSound& ClientLayer::RegisterSound(ClientLayerSound layerEntity)
	{
		std::size_t soundIndex = m_freeSoundIds.FindFirst();
		if (soundIndex == m_freeSoundIds.npos)
		{
			soundIndex = m_freeSoundIds.GetSize();
			m_freeSoundIds.Resize(soundIndex + 1, false);
			m_sounds.emplace_back();
		}
		else
			m_freeSoundIds.Reset(soundIndex);

		auto& soundOpt = m_sounds[soundIndex];
		assert(!soundOpt);
		soundOpt.emplace(std::move(layerEntity));

		OnSoundCreated(this, soundIndex, soundOpt->sound);

		return soundOpt->sound;
	}

	void ClientLayer::SyncVisuals()
	{
		ForEachLayerEntity([&](ClientLayerEntity& layerEntity)
		{
			layerEntity.SyncVisuals();
		});
	}

	void ClientLayer::CreateEntity(Nz::UInt32 entityId, const Packets::Helper::EntityData& entityData)
	{
		static std::string entityPrefix = "entity_";
		static std::string weaponPrefix = "weapon_";

		assert(m_isEnabled);

		ClientMatch& clientMatch = GetClientMatch();
		ClientEntityStore& entityStore = clientMatch.GetEntityStore();
		ClientWeaponStore& weaponStore = clientMatch.GetWeaponStore();
		const NetworkStringStore& networkStringStore = clientMatch.GetNetworkStringStore();

		const std::string& entityClass = networkStringStore.GetString(entityData.entityClass);

		PropertyValueMap properties;
		for (const auto& property : entityData.properties)
		{
			const std::string& propertyName = networkStringStore.GetString(property.name);
			properties.emplace(propertyName, property.value);
		}

		EntityId uniqueId = static_cast<EntityId>(entityData.uniqueId);

		const ClientLayerEntity* parent = nullptr;
		if (entityData.parentId)
		{
			auto idIt = m_serverEntityIds.find(entityData.parentId.value());
			//assert(idIt != m_serverEntityIds.end());
			if (idIt == m_serverEntityIds.end())
			{
				bwLog(GetMatch().GetLogger(), LogLevel::Error, "Entity #{} depends on {} which doesn't exist", uniqueId, entityData.parentId.value());
				return;
			}

			auto entityIt = m_entities.find(idIt->second);
			assert(entityIt != m_entities.end());

			parent = &entityIt.value().layerEntity;
		}

		std::optional<ClientLayerEntity> layerEntity;

		float scale = (entityData.scale) ? entityData.scale.value() : 1.f;

		//FIXME: Entity creation failure should instantiate some placeholder entity
		try
		{
			if (entityClass.compare(0, entityPrefix.size(), entityPrefix) == 0)
			{
				// Entity
				if (std::size_t elementIndex = entityStore.GetElementIndex(entityClass); elementIndex != ClientEntityStore::InvalidIndex)
				{
					auto entity = entityStore.InstantiateEntity(*this, elementIndex, entityId, uniqueId, entityData.position, entityData.rotation, scale, properties, (parent) ? parent->GetEntity() : entt::handle{});
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

		if (entityData.ownerPlayerIndex)
		{
			if (ClientPlayer* player = GetClientMatch().GetPlayerByIndex(*entityData.ownerPlayerIndex))
				layerEntity->GetEntity().emplace<ClientOwnerComponent>(player->CreateHandle());
		}

		if (entityData.health)
			layerEntity->InitializeHealth(entityData.health->maxHealth, entityData.health->currentHealth);

		if (entityData.physicsProperties)
		{
			auto& physProperties = *entityData.physicsProperties;

			if (Nz::RigidBody2DComponent* entityPhys = layerEntity->GetEntity().try_get<Nz::RigidBody2DComponent>())
			{
				entityPhys->SetMass(physProperties.mass, false);
				entityPhys->SetMomentOfInertia(physProperties.momentOfInertia);
				entityPhys->SetAngularVelocity(physProperties.angularVelocity);
				entityPhys->SetVelocity(physProperties.linearVelocity);

				if (physProperties.isAsleep)
					entityPhys->ForceSleep();
			}
			else
				bwLog(GetMatch().GetLogger(), LogLevel::Warning, "Entity {0} has physical properties but is not physical client-side");
		}

		RegisterEntity(std::move(layerEntity.value()));
	}

	void ClientLayer::HandleEntityDestruction(EntityId uniqueId)
	{
		std::size_t hash = m_entities.hash_function()(uniqueId);

		auto it = m_entities.find(uniqueId, hash);
		assert(it != m_entities.end());

		EntityData& entity = it.value();
		if (!entity.layerEntity.IsClientside())
		{
			std::size_t erasedCount = m_serverEntityIds.erase(entity.layerEntity.GetServerId());
			NazaraUnused(erasedCount);
			assert(erasedCount == 1);
		}

		OnEntityDelete(this, entity.layerEntity);

		// Don't trigger the Destroyed event on disabling layers
		if (IsEnabled())
		{
			if (ScriptComponent* scriptComponent = entity.layerEntity.GetEntity().try_get<ScriptComponent>())
				scriptComponent->ExecuteCallback<ElementEvent::Destroyed>();
		}
		// `it` and `entity` are no longer valid here (as a new entity could have been created by the destroyed callback)

		m_entities.erase(uniqueId, hash);
	}

	void ClientLayer::HandlePacket(const Packets::CreateEntities::Entity* entities, std::size_t entityCount)
	{
		assert(m_isEnabled);

		for (std::size_t i = 0; i < entityCount; ++i)
		{
			Nz::UInt32 entityId = entities[i].id;
			auto& entityData = entities[i].data;

			CreateEntity(entityId, entityData);
		}
	}

	void ClientLayer::HandlePacket(const Packets::DeleteEntities::Entity* entities, std::size_t entityCount)
	{
		assert(m_isEnabled);

		for (std::size_t i = 0; i < entityCount; ++i)
		{
			if (EntityId uniqueId = GetUniqueIdByServerId(entities[i].id); uniqueId != 0)
				HandleEntityDestruction(uniqueId);
		}
	}

	void ClientLayer::HandlePacket(const Packets::EnableLayer::Entity* entities, std::size_t entityCount)
	{
		assert(m_isEnabled);

		for (std::size_t i = 0; i < entityCount; ++i)
		{
			Nz::UInt32 entityId = entities[i].id;
			auto& entityData = entities[i].data;

			CreateEntity(entityId, entityData);
		}
	}

	void ClientLayer::HandlePacket(const Packets::EntitiesAnimation::Entity* entities, std::size_t entityCount)
	{
		assert(m_isEnabled);

		for (std::size_t i = 0; i < entityCount; ++i)
		{
			Nz::UInt32 entityId = entities[i].entityId;
			Nz::UInt8 animationId = entities[i].animId;

			auto entityOpt = GetEntityByServerId(entityId);
			if (!entityOpt)
				continue;

			ClientLayerEntity& localEntity = entityOpt.value();
			localEntity.UpdateAnimation(animationId);
		}
	}

	void ClientLayer::HandlePacket(const Packets::EntitiesDeath::Entity* entities, std::size_t entityCount)
	{
		assert(m_isEnabled);

		for (std::size_t i = 0; i < entityCount; ++i)
		{
			Nz::UInt32 entityId = entities[i].id;
			if (EntityId uniqueId = GetUniqueIdByServerId(entityId); uniqueId != 0)
			{
				auto entityOpt = GetEntity(uniqueId);
				assert(entityOpt);

				ClientLayerEntity& localEntity = entityOpt.value();
				if (localEntity.HasHealth())
					localEntity.UpdateHealth(0);
				else
					bwLog(GetMatch().GetLogger(), LogLevel::Error, "Received death event for entity {} which has no life", localEntity.GetUniqueId());

				HandleEntityDestruction(uniqueId);
			}
		}
	}

	void ClientLayer::HandlePacket(const Packets::EntitiesInputs::Entity* entities, std::size_t entityCount)
	{
		assert(m_isEnabled);

		for (std::size_t i = 0; i < entityCount; ++i)
		{
			Nz::UInt32 entityId = entities[i].id;
			const auto& inputs = entities[i].inputs;

			auto entityOpt = GetEntityByServerId(entityId);
			if (!entityOpt)
				continue;

			ClientLayerEntity& localEntity = entityOpt.value();
			localEntity.UpdateInputs(inputs);
		}
	}

	void ClientLayer::HandlePacket(const Packets::EntitiesScale::Entity* entities, std::size_t entityCount)
	{
		assert(m_isEnabled);

		for (std::size_t i = 0; i < entityCount; ++i)
		{
			Nz::UInt32 entityId = entities[i].id;
			float newScale = entities[i].newScale;

			auto entityOpt = GetEntityByServerId(entityId);
			if (!entityOpt)
				continue;

			ClientLayerEntity& localEntity = entityOpt.value();
			localEntity.UpdateScale(newScale);
		}
	}

	void ClientLayer::HandlePacket(const Packets::EntityPhysics& packet)
	{
		assert(packet.entityId.layerId == GetLayerIndex());

		auto entityOpt = GetEntityByServerId(packet.entityId.entityId);
		if (!entityOpt)
			return;

		ClientLayerEntity& localEntity = *entityOpt;
		if (localEntity.IsPhysical())
		{
			entt::handle entity = localEntity.GetEntity();

			auto& entityPhys = entity.get<Nz::RigidBody2DComponent>();
			entityPhys.SetMass(packet.mass, false);
			entityPhys.SetMomentOfInertia(packet.momentOfInertia);

			if (packet.asleep)
				entityPhys.ForceSleep();

			if (packet.playerMovement)
			{
				auto& packetPlayerMovement = packet.playerMovement.value();

				if (PlayerMovementComponent* entityPlayerMovement = entity.try_get<PlayerMovementComponent>())
				{
					entityPlayerMovement->UpdateJumpBoostHeight(packetPlayerMovement.jumpHeightBoost);
					entityPlayerMovement->UpdateJumpHeight(packetPlayerMovement.jumpHeight);
					entityPlayerMovement->UpdateMovementSpeed(packetPlayerMovement.movementSpeed);
				}
			}
		}
	}

	void ClientLayer::HandlePacket(const Packets::EntityWeapon& packet)
	{
		assert(packet.entityId.layerId == GetLayerIndex());

		auto entityOpt = GetEntityByServerId(packet.entityId.entityId);
		if (!entityOpt)
			return;

		ClientLayerEntity& localEntity = *entityOpt;
		if (packet.weaponEntityId != Packets::EntityWeapon::NoWeapon)
		{
			auto newWeaponOpt = GetEntityByServerId(packet.weaponEntityId);
			if (!newWeaponOpt)
				return;

			ClientLayerEntity& newWeapon = newWeaponOpt.value();
			localEntity.UpdateWeaponEntity(newWeapon.CreateHandle<ClientLayerEntity>());
		}
		else
			localEntity.UpdateWeaponEntity({});
	}

	void ClientLayer::HandlePacket(const Packets::HealthUpdate::Entity* entities, std::size_t entityCount)
	{
		assert(m_isEnabled);

		for (std::size_t i = 0; i < entityCount; ++i)
		{
			Nz::UInt32 entityId = entities[i].id;
			Nz::UInt16 currentHealth = entities[i].currentHealth;

			auto entityOpt = GetEntityByServerId(entityId);
			if (!entityOpt)
				continue;

			ClientLayerEntity& localEntity = entityOpt.value();
			if (localEntity.HasHealth())
				localEntity.UpdateHealth(currentHealth);
			else
				bwLog(GetMatch().GetLogger(), LogLevel::Error, "Received health data for entity {} which has none", localEntity.GetUniqueId());
		}
	}

	void ClientLayer::HandlePacket(const Packets::MapReset::Entity* entities, std::size_t entityCount)
	{
		assert(m_isEnabled);

		for (std::size_t i = 0; i < entityCount; ++i)
		{
			Nz::UInt32 entityId = entities[i].id;
			auto& entityData = entities[i].data;

			CreateEntity(entityId, entityData);
		}
	}
}
