// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_SHARED_TERRAIN_HPP
#define BURGWAR_SHARED_TERRAIN_HPP

#include <GameLibShared/MapData.hpp>
#include <GameLibShared/TerrainLayer.hpp>
#include <vector>

namespace bw
{
	class BurgApp;

	class Terrain
	{
		public:
			Terrain(BurgApp& app, MapData mapData);
			Terrain(const Terrain&) = delete;
			~Terrain() = default;

			inline TerrainLayer& GetLayer(std::size_t layerIndex);
			inline const TerrainLayer& GetLayer(std::size_t layerIndex) const;
			inline std::size_t GetLayerCount() const;
			inline const MapData& GetMapData() const;

			void Update(float elapsedTime);

			Terrain& operator=(const Terrain&) = delete;

		private:
			MapData m_mapData;
			std::vector<TerrainLayer> m_layers; //< Shouldn't resize because of raw pointer in Player
	};
}

#include <GameLibShared/Terrain.inl>

#endif
