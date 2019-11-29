// Copyright (C) 2016 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in Prerequisites.hpp

#include <CoreLib/Map.hpp>

namespace bw
{
	inline Map::Map() :
	m_isValid(false)
	{
	}

	inline Map::Map(MapInfo mapInfo) :
	m_isValid(true)
	{
		SetupDefault();
	}

	template<typename... Args> auto Map::AddLayer(Args&&... args) -> Layer&
	{
		return m_layers.emplace_back(std::forward<Args>(args)...);
	}

	inline auto Map::DropLayer(std::size_t i) -> Layer
	{
		Layer layer = std::move(GetLayer(i));
		m_layers.erase(m_layers.begin() + i);
		//TODO: Should the map fix entity properties?

		return layer;
	}

	template<typename... Args> auto Map::EmplaceLayer(std::size_t index, Args&&... args) -> Layer&
	{
		return *m_layers.emplace(m_layers.begin() + index, std::forward<Args>(args)...);
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

	inline auto Map::GetLayer(std::size_t i) -> Layer&
	{
		assert(i < m_layers.size());
		return m_layers[i];
	}

	inline auto Map::GetLayer(std::size_t i) const -> const Layer&
	{
		assert(i < m_layers.size());
		return m_layers[i];
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
