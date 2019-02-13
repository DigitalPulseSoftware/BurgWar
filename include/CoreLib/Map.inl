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

	inline Map::Map(const std::filesystem::path& folder) :
	m_isValid(false)
	{
		Load(folder);
		m_isValid = true;
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
}
