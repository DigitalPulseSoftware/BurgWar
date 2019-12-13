// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Terrain.hpp>
#include <cassert>

namespace bw
{
	inline TerrainLayer& Terrain::GetLayer(LayerIndex layerIndex)
	{
		assert(layerIndex < m_layers.size());
		return m_layers[layerIndex];
	}

	inline const TerrainLayer& Terrain::GetLayer(LayerIndex layerIndex) const
	{
		assert(layerIndex < m_layers.size());
		return m_layers[layerIndex];
	}

	inline LayerIndex Terrain::GetLayerCount() const
	{
		return LayerIndex(m_layers.size());
	}

	inline const Map& Terrain::GetMap() const
	{
		return m_map;
	}
}
