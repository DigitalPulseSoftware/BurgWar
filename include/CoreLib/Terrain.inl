// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Terrain.hpp>
#include <cassert>

namespace bw
{
	inline TerrainLayer& Terrain::GetLayer(std::size_t layerIndex)
	{
		assert(layerIndex < m_layers.size());
		return m_layers[layerIndex];
	}

	inline const TerrainLayer& Terrain::GetLayer(std::size_t layerIndex) const
	{
		assert(layerIndex < m_layers.size());
		return m_layers[layerIndex];
	}

	inline std::size_t Terrain::GetLayerCount() const
	{
		return m_layers.size();
	}

	inline const Map& Terrain::GetMap() const
	{
		return m_map;
	}
}
