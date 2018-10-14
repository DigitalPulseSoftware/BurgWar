// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/Terrain.hpp>

namespace bw
{
	inline TerrainLayer& Terrain::GetLayer(std::size_t layerIndex)
	{
		return m_layers[layerIndex];
	}

	inline const TerrainLayer& Terrain::GetLayer(std::size_t layerIndex) const
	{
		return m_layers[layerIndex];
	}
}
