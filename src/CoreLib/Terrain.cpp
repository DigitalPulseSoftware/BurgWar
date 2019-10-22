// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Terrain.hpp>

namespace bw
{
	Terrain::Terrain(Match& match, Map& map) :
	m_map(map)
	{
		m_layers.reserve(m_map.GetLayerCount());
		for (std::size_t layerIndex = 0; layerIndex < m_map.GetLayerCount(); ++layerIndex)
			m_layers.emplace_back(match, layerIndex, m_map.GetLayer(layerIndex));
	}

	void Terrain::Update(float elapsedTime)
	{
		for (TerrainLayer& layer : m_layers)
			layer.Update(elapsedTime);
	}
}
