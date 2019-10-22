// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_TERRAINLAYER_HPP
#define BURGWAR_CORELIB_TERRAINLAYER_HPP

#include <CoreLib/Map.hpp>
#include <CoreLib/SharedWorld.hpp>

namespace bw
{
	class BurgApp;
	class Match;

	class TerrainLayer
	{
		public:
			TerrainLayer(Match& match, std::size_t layerIndex, const Map::Layer& layerData);
			TerrainLayer(const TerrainLayer&) = delete;
			TerrainLayer(TerrainLayer&&) noexcept = default;
			~TerrainLayer() = default;

			inline SharedWorld& GetWorld();

			void Update(float elapsedTime);

			TerrainLayer& operator=(const TerrainLayer&) = delete;
			TerrainLayer& operator=(TerrainLayer&&) noexcept = default;

		private:
			SharedWorld m_world;
	};
}

#include <CoreLib/TerrainLayer.inl>

#endif
