// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Terrain.hpp>
#include <CoreLib/LayerIndex.hpp>

namespace bw
{
	Terrain::Terrain(Match& match, Map& map) :
	m_map(map)
	{
		m_layers.reserve(m_map.GetLayerCount());
		for (LayerIndex layerIndex = 0; layerIndex < m_map.GetLayerCount(); ++layerIndex)
			m_layers.emplace_back(match, LayerIndex(layerIndex), m_map.GetLayer(layerIndex));
	}

	void Terrain::Initialize()
	{
		for (TerrainLayer& layer : m_layers)
			layer.InitializeEntities();
	}

	void Terrain::Reset()
	{
		for (TerrainLayer& layer : m_layers)
			layer.ResetEntities();

		Initialize();
	}

	void Terrain::Update(float elapsedTime)
	{
		for (TerrainLayer& layer : m_layers)
			layer.TickUpdate(elapsedTime);
	}
}
