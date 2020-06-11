// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_TERRAINLAYER_HPP
#define BURGWAR_CORELIB_TERRAINLAYER_HPP

#include <CoreLib/Map.hpp>
#include <CoreLib/SharedLayer.hpp>

namespace bw
{
	class BurgApp;
	class Match;

	class TerrainLayer : public SharedLayer
	{
		friend class Terrain;

		public:
			TerrainLayer(Match& match, LayerIndex layerIndex, const Map::Layer& layerData);
			TerrainLayer(const TerrainLayer&) = delete;
			TerrainLayer(TerrainLayer&&) noexcept = default;
			~TerrainLayer() = default;

			Match& GetMatch();

			TerrainLayer& operator=(const TerrainLayer&) = delete;
			TerrainLayer& operator=(TerrainLayer&&) noexcept = default;

		private:
			void InitializeEntities();
	};
}

#include <CoreLib/TerrainLayer.inl>

#endif
