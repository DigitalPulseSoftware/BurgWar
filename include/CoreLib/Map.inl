// Copyright (C) 2016 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in Prerequisites.hpp

#include <CoreLib/Map.hpp>

namespace bw
{
	inline Map::Map() :
	m_freeUniqueId(0),
	m_isValid(false)
	{
	}

	inline Map::Map(MapInfo mapInfo) :
	m_freeUniqueId(0),
	m_mapInfo(std::move(mapInfo)),
	m_isValid(true)
	{
		SetupDefault();
	}

	template<typename... Args> 
	auto Map::AddEntity(std::size_t layerIndex, Args&&... args) -> Entity&
	{
		assert(IsValid());
		assert(layerIndex < m_layers.size());
		auto& layer = m_layers[layerIndex];

		std::size_t entityIndex = layer.entities.size();
		auto& newEntity = layer.entities.emplace_back(std::forward<Args>(args)...);
		newEntity.uniqueId = m_freeUniqueId++;

		RegisterEntity(newEntity.uniqueId, layerIndex, entityIndex);

		return newEntity;
	}

	template<typename... Args> 
	auto Map::AddLayer(Args&&... args) -> Layer&
	{
		std::size_t layerIndex = m_layers.size();
		Layer& layer = m_layers.emplace_back(std::forward<Args>(args)...);

		std::size_t entityIndex = 0;
		for (auto& entity : layer.entities)
		{
			entity.uniqueId = m_freeUniqueId++;

			RegisterEntity(entity.uniqueId, layerIndex, entityIndex);
			entityIndex++;
		}

		return layer;
	}

	inline auto Map::DropEntity(std::size_t layerIndex, std::size_t entityIndex) -> Entity
	{
		Layer& layer = GetLayer(layerIndex);

		assert(entityIndex < layer.entities.size());
		Entity entityData = std::move(layer.entities[entityIndex]);
		layer.entities.erase(layer.entities.begin() + entityIndex);

		UnregisterEntity(entityData.uniqueId);

		for (auto it = m_entitiesByUniqueId.begin(); it != m_entitiesByUniqueId.end(); ++it)
		{
			EntityIndices& entityIndices = it.value();
			if (entityIndices.layerIndex == layerIndex)
			{
				assert(entityIndices.entityIndex != entityIndex);
				if (entityIndices.entityIndex > entityIndex)
					entityIndices.entityIndex--;
			}
		}

		return entityData;
	}

	inline auto Map::DropLayer(std::size_t layerIndex) -> Layer
	{
		Layer layer = std::move(GetLayer(layerIndex));

		std::size_t entityIndex = 0;
		for (auto& entity : layer.entities)
		{
			UnregisterEntity(entity.uniqueId);
			entityIndex++;
		}

		for (auto it = m_entitiesByUniqueId.begin(); it != m_entitiesByUniqueId.end(); ++it)
		{
			EntityIndices& entityIndices = it.value();
			assert(entityIndices.layerIndex != layerIndex);
			if (entityIndices.layerIndex > layerIndex)
				entityIndices.layerIndex--;
		}

		m_layers.erase(m_layers.begin() + layerIndex);
		//TODO: Should the map fix entity properties?

		return layer;
	}

	template<typename ...Args>
	auto Map::EmplaceEntity(std::size_t layerIndex, std::size_t entityIndex, Args&&... args) -> Entity&
	{
		auto& layer = GetLayer(layerIndex);
		auto& entity = *layer.entities.emplace(layer.entities.begin() + entityIndex, std::forward<Args>(args)...);

		// If unique id is still in use, set a new one
		if (m_entitiesByUniqueId.find(entity.uniqueId) != m_entitiesByUniqueId.end())
			entity.uniqueId = m_freeUniqueId++;

		for (auto it = m_entitiesByUniqueId.begin(); it != m_entitiesByUniqueId.end(); ++it)
		{
			EntityIndices& entityIndices = it.value();
			if (entityIndices.layerIndex == layerIndex && entityIndices.entityIndex >= entityIndex)
				entityIndices.entityIndex++;
		}

		RegisterEntity(entity.uniqueId, layerIndex, entityIndex);

		return entity;
	}

	template<typename... Args> 
	auto Map::EmplaceLayer(std::size_t layerIndex, Args&&... args) -> Layer&
	{
		Layer& layer = *m_layers.emplace(m_layers.begin() + layerIndex, std::forward<Args>(args)...);

		for (auto it = m_entitiesByUniqueId.begin(); it != m_entitiesByUniqueId.end(); ++it)
		{
			EntityIndices& entityIndices = it.value();
			if (entityIndices.layerIndex >= layerIndex)
				entityIndices.layerIndex++;
		}

		std::size_t entityIndex = 0;
		for (auto& entity : layer.entities)
		{
			entity.uniqueId = m_freeUniqueId++;
			RegisterEntity(entity.uniqueId, layerIndex, entityIndex);

			entityIndex++;
		}

		return layer;
	}

	template<typename F>
	void Map::ForeachEntity(F&& func)
	{
		for (auto& layer : m_layers)
		{
			for (auto& entity : layer.entities)
				func(entity);
		}
	}

	inline Nz::Int64 Map::GenerateUniqueId()
	{
		return m_freeUniqueId++;
	}

	inline auto Map::GetAssets() -> std::vector<Asset>&
	{
		return m_assets;
	}

	inline auto Map::GetAssets() const -> const std::vector<Asset>&
	{
		return m_assets;
	}

	inline auto Map::GetEntity(std::size_t layerIndex, std::size_t entityIndex) -> Entity&
	{
		auto& layer = GetLayer(layerIndex);
		assert(entityIndex < layer.entities.size());
		return layer.entities[entityIndex];
	}

	inline auto Map::GetEntity(std::size_t layerIndex, std::size_t entityIndex) const -> const Entity&
	{
		auto& layer = GetLayer(layerIndex);
		assert(entityIndex < layer.entities.size());
		return layer.entities[entityIndex];
	}

	inline auto Map::GetEntity(Nz::Int64 uniqueId) -> Entity&
	{
		const EntityIndices& indices = GetEntityIndices(uniqueId);
		return GetEntity(indices.layerIndex, indices.entityIndex);
	}

	inline auto Map::GetEntity(Nz::Int64 uniqueId) const -> const Entity&
	{
		const EntityIndices& indices = GetEntityIndices(uniqueId);
		return GetEntity(indices.layerIndex, indices.entityIndex);
	}

	inline auto Map::GetEntityIndices(Nz::Int64 uniqueId) const -> const EntityIndices&
	{
		auto it = m_entitiesByUniqueId.find(uniqueId);
		assert(it != m_entitiesByUniqueId.end());

		return it->second;
	}

	inline std::size_t Map::GetEntityCount(std::size_t layerIndex) const
	{
		auto& layer = GetLayer(layerIndex);
		return layer.entities.size();
	}

	inline Nz::Int64 Map::GetFreeUniqueId() const
	{
		return m_freeUniqueId;
	}

	inline auto Map::GetLayer(std::size_t layerIndex) -> Layer&
	{
		assert(layerIndex < m_layers.size());
		return m_layers[layerIndex];
	}

	inline auto Map::GetLayer(std::size_t layerIndex) const -> const Layer&
	{
		assert(layerIndex < m_layers.size());
		return m_layers[layerIndex];
	}

	inline std::size_t Map::GetLayerCount() const
	{
		return m_layers.size();
	}

	inline const MapInfo& Map::GetMapInfo() const
	{
		return m_mapInfo;
	}

	inline bool Map::IsValid() const
	{
		return m_isValid;
	}

	inline auto Map::MoveEntity(std::size_t sourceLayerIndex, std::size_t sourceEntityIndex, std::size_t targetLayerIndex, std::size_t targetEntityIndex) -> Entity&
	{
		assert(sourceLayerIndex != targetLayerIndex);

		auto& sourceLayer = GetLayer(sourceLayerIndex);
		assert(sourceEntityIndex < sourceLayer.entities.size());
		auto& targetLayer = GetLayer(targetLayerIndex);
		assert(targetEntityIndex <= targetLayer.entities.size());

		Entity& sourceEntity = sourceLayer.entities[sourceEntityIndex];

		auto movedIt = m_entitiesByUniqueId.find(sourceEntity.uniqueId);
		assert(movedIt != m_entitiesByUniqueId.end());

		targetLayer.entities.emplace(targetLayer.entities.begin() + targetEntityIndex, std::move(sourceEntity));
		sourceLayer.entities.erase(sourceLayer.entities.begin() + sourceEntityIndex);

		for (auto it = m_entitiesByUniqueId.begin(); it != m_entitiesByUniqueId.end(); ++it)
		{
			EntityIndices& entityIndices = it.value();
			if (entityIndices.layerIndex == sourceLayerIndex)
			{
				if (entityIndices.entityIndex > sourceEntityIndex)
					entityIndices.entityIndex--;
			}
			else if (entityIndices.layerIndex == targetLayerIndex)
			{
				if (entityIndices.entityIndex >= targetEntityIndex)
					entityIndices.entityIndex++;
			}
		}

		movedIt.value() = EntityIndices{ targetLayerIndex, targetEntityIndex };

		return targetLayer.entities[targetEntityIndex];
	}

	inline void Map::SwapEntities(std::size_t layerIndex, std::size_t firstEntityIndex, std::size_t secondEntityIndex)
	{
		if (firstEntityIndex == secondEntityIndex)
			return;

		Entity& firstEntity = GetEntity(layerIndex, firstEntityIndex);
		Entity& secondEntity = GetEntity(layerIndex, secondEntityIndex);

		std::swap(firstEntity, secondEntity);

		// Fix uniqueId
		for (auto it = m_entitiesByUniqueId.begin(); it != m_entitiesByUniqueId.end(); ++it)
		{
			EntityIndices& entityIndices = it.value();
			if (entityIndices.layerIndex == layerIndex)
			{
				if (entityIndices.entityIndex == firstEntityIndex)
					entityIndices.entityIndex = secondEntityIndex;
				else if (entityIndices.entityIndex == secondEntityIndex)
					entityIndices.entityIndex = firstEntityIndex;
			}
		}
	}

	inline void Map::SwapLayers(std::size_t firstLayerIndex, std::size_t secondLayerIndex)
	{
		Layer& firstLayer = GetLayer(firstLayerIndex);
		Layer& secondLayer = GetLayer(secondLayerIndex);

		if (firstLayerIndex == secondLayerIndex)
			return;

		std::swap(firstLayer, secondLayer);

		// Fix uniqueId
		for (auto it = m_entitiesByUniqueId.begin(); it != m_entitiesByUniqueId.end(); ++it)
		{
			EntityIndices& entityIndices = it.value();
			if (entityIndices.layerIndex == firstLayerIndex)
				entityIndices.layerIndex = secondLayerIndex;
			else if (entityIndices.layerIndex == secondLayerIndex)
				entityIndices.layerIndex = firstLayerIndex;
		}
	}

	inline Map Map::LoadFromBinary(const std::filesystem::path& mapFile)
	{
		Map map;
		map.LoadFromBinaryInternal(mapFile);

		return map;
	}

	inline Map Map::LoadFromFolder(const std::filesystem::path& folder)
	{
		Map map;
		map.LoadFromTextInternal(folder);

		return map;
	}

	inline void Map::RegisterEntity(Nz::Int64 uniqueId, std::size_t layerIndex, std::size_t entityIndex)
	{
		assert(m_entitiesByUniqueId.find(uniqueId) == m_entitiesByUniqueId.end());
		m_entitiesByUniqueId[uniqueId] = EntityIndices{ layerIndex, entityIndex };
	}

	inline void Map::UnregisterEntity(Nz::Int64 uniqueId)
	{
		assert(m_entitiesByUniqueId.find(uniqueId) != m_entitiesByUniqueId.end());
		m_entitiesByUniqueId.erase(uniqueId);
	}
}
