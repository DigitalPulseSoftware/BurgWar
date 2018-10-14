// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/Terrain.hpp>

namespace bw
{
	Terrain::Terrain(std::size_t layerCount)
	{
		m_layers.reserve(layerCount);
		for (std::size_t i = 0; i < layerCount; ++i)
			m_layers.emplace_back();
	}

	void Terrain::Update(float elapsedTime)
	{
		for (TerrainLayer& layer : m_layers)
			layer.Update(elapsedTime);
	}
}
