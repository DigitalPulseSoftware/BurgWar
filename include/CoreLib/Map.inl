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
		auto& newEntity = layer.entities.emplace_back(std::forward<Args>(args)...);
		newEntity.uniqueId = m_freeUniqueId++;

		return newEntity;
	}

	template<typename... Args> 
	auto Map::AddLayer(Args&&... args) -> Layer&
	{
		Layer& layer = m_layers.emplace_back(std::forward<Args>(args)...);
		for (auto& entity : layer.entities)
			entity.uniqueId = m_freeUniqueId++;

		return layer;
	}

	inline auto Map::DropLayer(std::size_t layerIndex) -> Layer
	{
		Layer layer = std::move(GetLayer(layerIndex));
		m_layers.erase(m_layers.begin() + layerIndex);
		//TODO: Should the map fix entity properties?

		return layer;
	}

	template<typename ...Args>
	auto Map::EmplaceEntity(std::size_t layerIndex, std::size_t entityIndex, Args&&... args) -> Entity&
	{
		auto& layer = GetLayer(layerIndex);
		auto& entity = *layer.entities.emplace(layer.entities.begin() + entityIndex, std::forward<Args>(args)...);
		entity.uniqueId = m_freeUniqueId++;

		return entity;
	}

	template<typename... Args> 
	auto Map::EmplaceLayer(std::size_t index, Args&&... args) -> Layer&
	{
		Layer& layer = *m_layers.emplace(m_layers.begin() + index, std::forward<Args>(args)...);
		for (auto& entity : layer.entities)
			entity.uniqueId = m_freeUniqueId++;

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

	inline auto Map::MoveEntity(std::size_t sourceLayerIndex, std::size_t sourceEntityIndex, std::size_t targetLayerIndex) -> Entity&
	{
		auto& sourceLayer = GetLayer(sourceLayerIndex);
		assert(sourceEntityIndex < sourceLayer.entities.size());
		auto& targetLayer = GetLayer(targetLayerIndex);
		targetLayer.entities.emplace_back(std::move(sourceLayer.entities[sourceEntityIndex]));
		sourceLayer.entities.erase(sourceLayer.entities.begin() + sourceEntityIndex);

		return targetLayer.entities.back();
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
}
