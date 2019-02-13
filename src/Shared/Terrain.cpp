// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/Terrain.hpp>

namespace bw
{
	Terrain::Terrain(BurgApp& app, Match& match, MapData mapData, Map mapData2) :
	m_mapData(std::move(mapData))
	{
		m_layers.reserve(mapData2.GetLayerCount());
		for (std::size_t layerIndex = 0; layerIndex < mapData2.GetLayerCount(); ++layerIndex)
			m_layers.emplace_back(app, match, m_mapData.layers[layerIndex], mapData2.GetLayer(layerIndex), m_mapData.tileSize);
	}

	void Terrain::Update(float elapsedTime)
	{
		for (TerrainLayer& layer : m_layers)
			layer.Update(elapsedTime);
	}
}
