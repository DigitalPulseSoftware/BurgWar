// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <GameLibShared/Terrain.hpp>

namespace bw
{
	Terrain::Terrain(BurgApp& app, MapData mapData) :
	m_mapData(std::move(mapData))
	{
		m_layers.reserve(m_mapData.layers.size());
		for (auto& layer : m_mapData.layers)
			m_layers.emplace_back(app, layer, m_mapData.tileSize);
	}

	void Terrain::Update(float elapsedTime)
	{
		for (TerrainLayer& layer : m_layers)
			layer.Update(elapsedTime);
	}
}
