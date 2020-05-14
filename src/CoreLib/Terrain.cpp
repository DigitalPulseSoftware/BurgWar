// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Terrain.hpp>
#include <CoreLib/LayerIndex.hpp>

namespace bw
{
	Terrain::Terrain(Map& map) :
	m_map(map)
	{
	}

	void Terrain::Initialize(Match& match)
	{
		m_layers.reserve(m_map.GetLayerCount());
		for (std::size_t layerIndex = 0; layerIndex < m_map.GetLayerCount(); ++layerIndex)
			m_layers.emplace_back(match, LayerIndex(layerIndex), m_map.GetLayer(layerIndex));

		for (TerrainLayer& layer : m_layers)
			layer.InitializeEntities();
	}

	void Terrain::Update(float elapsedTime)
	{
		for (TerrainLayer& layer : m_layers)
			layer.TickUpdate(elapsedTime);
	}
}
