// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_TERRAIN_HPP
#define BURGWAR_CORELIB_TERRAIN_HPP

#include <CoreLib/Map.hpp>
#include <CoreLib/TerrainLayer.hpp>
#include <vector>

namespace bw
{
	class Match;

	class Terrain
	{
		public:
			Terrain(Map& map);
			Terrain(const Terrain&) = delete;
			~Terrain() = default;

			inline TerrainLayer& GetLayer(LayerIndex layerIndex);
			inline const TerrainLayer& GetLayer(LayerIndex layerIndex) const;
			inline LayerIndex GetLayerCount() const;
			inline const Map& GetMap() const;

			void Initialize(Match& match);

			void Update(float elapsedTime);

			Terrain& operator=(const Terrain&) = delete;

		private:
			Map& m_map;
			std::vector<TerrainLayer> m_layers; //< Shouldn't resize because of raw pointer in Player
	};
}

#include <CoreLib/Terrain.inl>

#endif
