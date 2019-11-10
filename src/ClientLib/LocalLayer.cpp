// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/LocalLayer.hpp>
#include <ClientLib/ClientSession.hpp>
#include <ClientLib/LocalMatch.hpp>
#include <ClientLib/Components/LayerEntityComponent.hpp>
#include <ClientLib/Scripting/ClientEntityStore.hpp>
#include <ClientLib/Scripting/ClientWeaponStore.hpp>
#include <NDK/Components/NodeComponent.hpp>

namespace bw
{
	LocalLayer::LocalLayer(LocalMatch& match, LayerIndex layerIndex, const Nz::Color& backgroundColor) :
	SharedLayer(match, layerIndex),
	m_backgroundColor(backgroundColor),
	m_isEnabled(false)
	{
	}

	void LocalLayer::Enable(bool enable)
	{
		if (m_isEnabled == enable)
			return;

		m_isEnabled = enable;

		if (!enable)
			m_serverEntityIdToClient.clear();
	}

	LocalMatch& LocalLayer::GetLocalMatch()
	{
		return static_cast<LocalMatch&>(SharedLayer::GetMatch());
	}

	void LocalLayer::SyncVisuals()
	{
		ForEachLayerEntity([&](LocalLayerEntity& layerEntity)
		{
			layerEntity.SyncVisuals();
		});
	}
	
	void LocalLayer::CreateEntity(Nz::UInt32 entityId, const Packets::Helper::EntityData& entityData)
	{
		static std::string entityPrefix = "entity_";
		static std::string weaponPrefix = "weapon_";

		assert(m_isEnabled);

		Ndk::World& world = GetWorld();

		LocalMatch& localMatch = GetLocalMatch();
		ClientEntityStore& entityStore = localMatch.GetEntityStore();
		ClientWeaponStore& weaponStore = localMatch.GetWeaponStore();
		const NetworkStringStore& networkStringStore = localMatch.GetClientSession().GetNetworkStringStore();

		const std::string& entityClass = networkStringStore.GetString(entityData.entityClass);
		bwLog(GetMatch().GetLogger(), LogLevel::Debug, "Creating entity {} on layer {} of type {}", entityId, GetLayerIndex(), entityClass);

		EntityProperties properties;
		for (const auto& property : entityData.properties)
		{
			const std::string& propertyName = networkStringStore.GetString(property.name);

			std::visit([&](auto&& value)
			{
				using T = std::decay_t<decltype(value)>;

				if constexpr (std::is_same_v<T, std::vector<bool>> ||
				              std::is_same_v<T, std::vector<float>> ||
				              std::is_same_v<T, std::vector<Nz::Int64>> ||
				              std::is_same_v<T, std::vector<Nz::Vector2f>> ||
				              std::is_same_v<T, std::vector<Nz::Vector2i64>> ||
				              std::is_same_v<T, std::vector<std::string>>)
				{
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
				}
				else
					static_assert(AlwaysFalse<T>::value, "non-exhaustive visitor");

			}, property.value);
		}

		/*const*/ LocalLayerEntity* parent = nullptr;
		if (entityData.parentId)
		{
			auto it = m_serverEntityIdToClient.find(entityData.parentId.value());
			assert(it != m_serverEntityIdToClient.end());

			//parent = &it->second;
			parent = &it.value();
		}

		std::optional<LocalLayerEntity> layerEntity;

		Ndk::EntityHandle entity;
		if (entityClass.compare(0, entityPrefix.size(), entityPrefix) == 0)
		{
			// Entity
			if (std::size_t elementIndex = entityStore.GetElementIndex(entityClass); elementIndex != ClientEntityStore::InvalidIndex)
			{
				auto entity = entityStore.InstantiateEntity(*this, elementIndex, entityId, entityData.position, entityData.rotation, properties, entityData.physicsProperties.has_value());
				if (!entity)
					return;

				layerEntity.emplace(std::move(entity.value()));
			}
		}
		else if (entityClass.compare(0, weaponPrefix.size(), weaponPrefix) == 0)
		{
			// Weapon
			if (std::size_t weaponIndex = weaponStore.GetElementIndex(entityClass); weaponIndex != ClientEntityStore::InvalidIndex)
			{
				assert(parent);

				auto weapon = weaponStore.InstantiateWeapon(*this, weaponIndex, entityId, properties, parent->GetEntity());
				if (!weapon)
					return;

				weapon->Disable(); //< Disable weapon entities by default

				layerEntity.emplace(std::move(weapon.value()));
			}
		}
		else
		{
			// Unknown
			bwLog(GetMatch().GetLogger(), LogLevel::Error, "Failed to decode entity type: {0}", entityClass);
			return;
		}

		//if (m_debug)
		//	CreateGhostEntity(serverEntity);

		if (entityData.health)
			layerEntity->InitializeHealth(entityData.health->maxHealth, entityData.health->currentHealth);

		if (entityData.name)
			layerEntity->InitializeName(entityData.name.value());

		assert(m_serverEntityIdToClient.find(entityId) == m_serverEntityIdToClient.end());
		auto it = m_serverEntityIdToClient.emplace(entityId, std::move(layerEntity.value())).first;

		OnEntityCreated(this, it.value());
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

			bwLog(GetMatch().GetLogger(), LogLevel::Debug, "Deleting entity {} on layer {}", entityId, GetLayerIndex());

			auto it = m_serverEntityIdToClient.find(entityId);
			//assert(it != m_serverEntityIdToClient.end());
			if (it == m_serverEntityIdToClient.end())
				continue;

			OnEntityDelete(this, it.value());

			//m_prediction->DeleteEntity(BuildEntityId(entityData.id.layerId, it->second.entity->GetId()));
			m_serverEntityIdToClient.erase(it);
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

			auto it = m_serverEntityIdToClient.find(entityId);
			if (it == m_serverEntityIdToClient.end())
				continue;

			LocalLayerEntity& localEntity = it.value();
			localEntity.UpdateAnimation(animationId);
		}
	}

	void LocalLayer::HandlePacket(const Packets::EntitiesDeath::Entity* entities, std::size_t entityCount)
	{
		assert(m_isEnabled);

		for (std::size_t i = 0; i < entityCount; ++i)
		{
			Nz::UInt32 entityId = entities[i].id;

			auto it = m_serverEntityIdToClient.find(entityId);
			if (it == m_serverEntityIdToClient.end())
				continue;

			LocalLayerEntity& localEntity = it.value();
			localEntity.UpdateHealth(0);
		}
	}

	void LocalLayer::HandlePacket(const Packets::EntitiesInputs::Entity* entities, std::size_t entityCount)
	{
		assert(m_isEnabled);

		for (std::size_t i = 0; i < entityCount; ++i)
		{
			Nz::UInt32 entityId = entities[i].id;
			const auto& inputs = entities[i].inputs;

			auto it = m_serverEntityIdToClient.find(entityId);
			if (it == m_serverEntityIdToClient.end())
				continue;

			LocalLayerEntity& localEntity = it.value();
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

			auto it = m_serverEntityIdToClient.find(entityId);
			if (it == m_serverEntityIdToClient.end())
				continue;

			LocalLayerEntity& localEntity = it.value();
			localEntity.UpdateHealth(currentHealth);
		}
	}

	void LocalLayer::HandlePacket(const Packets::MatchState::Entity* entities, std::size_t entityCount)
	{
		assert(m_isEnabled);

		for (std::size_t i = 0; i < entityCount; ++i)
		{
			auto& entityData = entities[i];

			auto it = m_serverEntityIdToClient.find(entityData.id);
			if (it == m_serverEntityIdToClient.end())
				continue;

			LocalLayerEntity& localEntity = it.value();
			if (localEntity.IsPhysical())
			{
				assert(entityData.physicsProperties.has_value());
				auto& physData = entityData.physicsProperties.value();
				localEntity.UpdateState(entityData.position, entityData.rotation, physData.linearVelocity, physData.angularVelocity);
			}
			else
			{
				assert(!entityData.physicsProperties.has_value());
				localEntity.UpdateState(entityData.position, entityData.rotation);
			}

			if (entityData.playerMovement)
				localEntity.UpdatePlayerMovement(entityData.playerMovement->isFacingRight);
		}
	}
}
