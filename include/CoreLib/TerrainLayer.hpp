// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_TERRAINLAYER_HPP
#define BURGWAR_CORELIB_TERRAINLAYER_HPP

#include <CoreLib/Map.hpp>
#include <CoreLib/Match.hpp>
#include <CoreLib/SharedLayer.hpp>

namespace bw
{
	class BurgApp;
	class Match;

	class TerrainLayer : public SharedLayer
	{
		public:
			TerrainLayer(Match& match, LayerIndex layerIndex, const Map::Layer& layerData);
			TerrainLayer(const TerrainLayer&) = delete;
			TerrainLayer(TerrainLayer&&) noexcept = default;
			~TerrainLayer() = default;

			inline Match& GetMatch();

			TerrainLayer& operator=(const TerrainLayer&) = delete;
			TerrainLayer& operator=(TerrainLayer&&) noexcept = default;
	};
}

#include <CoreLib/TerrainLayer.inl>

#endif
