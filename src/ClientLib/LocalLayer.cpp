// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/LocalLayer.hpp>
#include <ClientLib/ClientSession.hpp>
#include <ClientLib/LocalMatch.hpp>
#include <ClientLib/Components/LayerEntityComponent.hpp>
#include <ClientLib/Systems/FrameCallbackSystem.hpp>
#include <ClientLib/Systems/PostFrameCallbackSystem.hpp>
#include <ClientLib/Systems/VisualInterpolationSystem.hpp>
#include <ClientLib/Scripting/ClientEntityStore.hpp>
#include <ClientLib/Scripting/ClientWeaponStore.hpp>
#include <NDK/Components/NodeComponent.hpp>

namespace bw
{
	LocalLayer::LocalLayer(LocalMatch& match, LayerIndex layerIndex, const Nz::Color& backgroundColor) :
	SharedLayer(match, layerIndex),
	m_backgroundColor(backgroundColor),
	m_isEnabled(false),
	m_isPredictionEnabled(false)
	{
		Ndk::World& world = GetWorld();
		world.AddSystem<FrameCallbackSystem>(match);
		world.AddSystem<PostFrameCallbackSystem>(match);
		world.AddSystem<VisualInterpolationSystem>();
	}

	LocalLayer::~LocalLayer()
	{
		// Kill all entities while still alive
		GetWorld().Clear();
	}

	LocalLayer::LocalLayer(LocalLayer&& layer) noexcept :
	SharedLayer(std::move(layer)),
	m_clientEntities(std::move(layer.m_clientEntities)),
	m_serverEntities(std::move(layer.m_serverEntities)),
	m_backgroundColor(layer.m_backgroundColor),
	m_isEnabled(layer.m_isEnabled),
	m_isPredictionEnabled(layer.m_isPredictionEnabled)
	{
		for (auto it = m_clientEntities.begin(); it != m_clientEntities.end(); ++it)
		{
			EntityData& clientEntity = it.value();
			clientEntity.onDestruction.Connect(clientEntity.layerEntity.GetEntity()->OnEntityDestruction, this, &LocalLayer::HandleClientEntityDestruction);
		}

		for (auto it = m_serverEntities.begin(); it != m_serverEntities.end(); ++it)
		{
			EntityData& serverEntity = it.value();
			serverEntity.onDestruction.Connect(serverEntity.layerEntity.GetEntity()->OnEntityDestruction, [this, serverId = serverEntity.layerEntity.GetServerId()](Ndk::Entity*)
			{
				HandleServerEntityDestruction(serverId);
			});
		}
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
			m_clientEntities.clear();
			m_serverEntities.clear();

			// Since we are disabled, refresh won't be called until we are enabled, refresh the world now to kill entities
			GetWorld().Refresh();
		}
	}

	LocalMatch& LocalLayer::GetLocalMatch()
	{
		return static_cast<LocalMatch&>(SharedLayer::GetMatch());
	}

	void LocalLayer::FrameUpdate(float elapsedTime)
	{
		Ndk::World& world = GetWorld();
		world.ForEachSystem([](Ndk::BaseSystem& system)
		{
			system.Enable(false);
		});

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
		const Ndk::EntityHandle& entity = layerEntity.GetEntity();
		assert(entity);
		assert(entity->GetWorld() == &GetWorld());

		if (layerEntity.GetServerId() == LocalLayerEntity::ClientsideId)
		{
			assert(m_clientEntities.find(entity->GetId()) == m_clientEntities.end());
			auto it = m_clientEntities.emplace(entity->GetId(), std::move(layerEntity)).first;
			// Warning: entity reference is invalidated from here

			EntityData& clientEntity = it.value();

			clientEntity.onDestruction.Connect(clientEntity.layerEntity.GetEntity()->OnEntityDestruction, this, &LocalLayer::HandleClientEntityDestruction);

			OnEntityCreated(this, clientEntity.layerEntity);

			return clientEntity.layerEntity;
		}
		else
		{
			Nz::UInt32 serverId = layerEntity.GetServerId();
			assert(m_serverEntities.find(serverId) == m_serverEntities.end());
			auto it = m_serverEntities.emplace(serverId, std::move(layerEntity)).first;
			// Warning: entity reference is invalidated from here

			EntityData& serverEntity = it.value();

			serverEntity.onDestruction.Connect(serverEntity.layerEntity.GetEntity()->OnEntityDestruction, [this, serverId = serverEntity.layerEntity.GetServerId()](Ndk::Entity*)
			{
				HandleServerEntityDestruction(serverId);
			});

			OnEntityCreated(this, serverEntity.layerEntity);

			return serverEntity.layerEntity;
		}
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
		const NetworkStringStore& networkStringStore = localMatch.GetClientSession().GetNetworkStringStore();

		const std::string& entityClass = networkStringStore.GetString(entityData.entityClass);

		EntityProperties properties;
		for (const auto& property : entityData.properties)
		{
			const std::string& propertyName = networkStringStore.GetString(property.name);

			std::visit([&](auto&& value)
			{
				using T = std::decay_t<decltype(value)>;
				using StoredType = typename T::value_type;

				if (property.isArray)
				{
					EntityPropertyArray<StoredType> elements(value.size());
					for (std::size_t i = 0; i < value.size(); ++i)
						elements[i] = value[i];

					properties.emplace(propertyName, std::move(elements));
				}
				else
					properties.emplace(propertyName, value.front());

			}, property.value);
		}

		const LocalLayerEntity* parent = nullptr;
		if (entityData.parentId)
		{
			auto it = m_serverEntities.find(entityData.parentId.value());
			assert(it != m_serverEntities.end());

			parent = &it.value().layerEntity;
		}

		Nz::Int64 uniqueId = static_cast<Nz::Int64>(entityData.uniqueId);

		std::optional<LocalLayerEntity> layerEntity;

		//FIXME: Entity creation failure should instantiate some placeholder entity
		try
		{
			if (entityClass.compare(0, entityPrefix.size(), entityPrefix) == 0)
			{
				// Entity
				if (std::size_t elementIndex = entityStore.GetElementIndex(entityClass); elementIndex != ClientEntityStore::InvalidIndex)
				{
					auto entity = entityStore.InstantiateEntity(*this, elementIndex, entityId, uniqueId, entityData.position, entityData.rotation, properties, (parent) ? parent->GetEntity() : Ndk::EntityHandle::InvalidHandle);
					if (!entity)
					{
						bwLog(GetMatch().GetLogger(), LogLevel::Error, "Failed to instantiate entity {} of type {}", entityId, entityClass);
						return;
					}

					layerEntity.emplace(std::move(entity.value()));
				}
				else
				{
					bwLog(GetMatch().GetLogger(), LogLevel::Error, "Unregistered entity type: {0}", entityClass);
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
						return;

					weapon->Disable(); //< Disable weapon entities by default

					layerEntity.emplace(std::move(weapon.value()));
				}
				else
				{
					bwLog(GetMatch().GetLogger(), LogLevel::Error, "Unregistered entity type: {0}", entityClass);
					return;
				}
			}
			else
			{
				// Unknown
				bwLog(GetMatch().GetLogger(), LogLevel::Error, "Failed to decode entity type: {0}", entityClass);
				return;
			}
		}
		catch (const std::exception & e)
		{
			bwLog(GetMatch().GetLogger(), LogLevel::Error, "Failed to instantiate entity {} of type {}: {}", entityId, entityClass, e.what());
			return;
		}

		if (entityData.health)
			layerEntity->InitializeHealth(entityData.health->maxHealth, entityData.health->currentHealth);

		if (entityData.name)
			layerEntity->InitializeName(entityData.name.value());

		RegisterEntity(std::move(layerEntity.value()));
	}

	void LocalLayer::HandleClientEntityDestruction(Ndk::Entity* entity)
	{
		auto it = m_clientEntities.find(entity->GetId());
		assert(it != m_clientEntities.end());

		OnEntityDelete(this, it.value().layerEntity);
		m_clientEntities.erase(it);
	}

	void LocalLayer::HandleServerEntityDestruction(Nz::UInt32 serverId)
	{
		auto it = m_serverEntities.find(serverId);
		assert(it != m_serverEntities.end());

		OnEntityDelete(this, it.value().layerEntity);
		m_serverEntities.erase(it);
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
			Nz::UInt32 entityId = entities[i].id;

			auto it = m_serverEntities.find(entityId);
			if (it == m_serverEntities.end())
				continue;

			OnEntityDelete(this, it.value().layerEntity);

			m_serverEntities.erase(it);
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

			auto it = m_serverEntities.find(entityId);
			if (it == m_serverEntities.end())
				continue;

			LocalLayerEntity& localEntity = it.value().layerEntity;
			localEntity.UpdateAnimation(animationId);
		}
	}

	void LocalLayer::HandlePacket(const Packets::EntitiesDeath::Entity* entities, std::size_t entityCount)
	{
		assert(m_isEnabled);

		for (std::size_t i = 0; i < entityCount; ++i)
		{
			Nz::UInt32 entityId = entities[i].id;

			auto it = m_serverEntities.find(entityId);
			if (it == m_serverEntities.end())
				continue;

			LocalLayerEntity& localEntity = it.value().layerEntity;
			if (localEntity.HasHealth())
				localEntity.UpdateHealth(0);
			else
				bwLog(GetMatch().GetLogger(), LogLevel::Error, "Received death event for entity {} which has no life", localEntity.GetUniqueId());

			OnEntityDelete(this, it.value().layerEntity);

			m_serverEntities.erase(it);
		}
	}

	void LocalLayer::HandlePacket(const Packets::EntitiesInputs::Entity* entities, std::size_t entityCount)
	{
		assert(m_isEnabled);

		for (std::size_t i = 0; i < entityCount; ++i)
		{
			Nz::UInt32 entityId = entities[i].id;
			const auto& inputs = entities[i].inputs;

			auto it = m_serverEntities.find(entityId);
			if (it == m_serverEntities.end())
				continue;

			LocalLayerEntity& localEntity = it.value().layerEntity;
			localEntity.UpdateInputs(inputs);
		}
	}

	void LocalLayer::HandlePacket(const Packets::HealthUpdate::Entity* entities, std::size_t entityCount)
	{
		assert(m_isEnabled);

		for (std::size_t i = 0; i < entityCount; ++i)
		{
			Nz::UInt32 entityId = entities[i].id;
			Nz::UInt16 currentHealth = entities[i].currentHealth;

			auto it = m_serverEntities.find(entityId);
			if (it == m_serverEntities.end())
				continue;

			LocalLayerEntity& localEntity = it.value().layerEntity;
			if (localEntity.HasHealth())
				localEntity.UpdateHealth(currentHealth);
			else
				bwLog(GetMatch().GetLogger(), LogLevel::Error, "Received health data for entity {} which has none", localEntity.GetUniqueId());
		}
	}

	void LocalLayer::HandlePacket(const Packets::MatchState::Entity* entities, std::size_t entityCount)
	{
		assert(m_isEnabled);

		for (std::size_t i = 0; i < entityCount; ++i)
		{
			auto& entityData = entities[i];

			auto it = m_serverEntities.find(entityData.id);
			if (it == m_serverEntities.end())
				continue;

			LocalLayerEntity& localEntity = it.value().layerEntity;
			if (localEntity.IsPhysical())
			{
				if (entityData.physicsProperties.has_value())
				{
					auto& physData = entityData.physicsProperties.value();
					localEntity.UpdateState(entityData.position, entityData.rotation, physData.linearVelocity, physData.angularVelocity);
				}
				else
				{
					bwLog(GetMatch().GetLogger(), LogLevel::Warning, "Entity {} has client-side physics but server sends no data", localEntity.GetUniqueId());
					localEntity.UpdateState(entityData.position, entityData.rotation);
				}
			}
			else
			{
				if (entityData.physicsProperties.has_value())
					bwLog(GetMatch().GetLogger(), LogLevel::Warning, "Received physics properties for entity {} which is not physical client-side", localEntity.GetUniqueId());

				localEntity.UpdateState(entityData.position, entityData.rotation);
			}

			if (entityData.playerMovement)
				localEntity.UpdatePlayerMovement(entityData.playerMovement->isFacingRight);
		}
	}
}
