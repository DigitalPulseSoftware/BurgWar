// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_TERRAIN_HPP
#define BURGWAR_CORELIB_TERRAIN_HPP

#include <CoreLib/Export.hpp>
#include <CoreLib/Map.hpp>
#include <CoreLib/TerrainLayer.hpp>
#include <vector>

namespace bw
{
	class Match;

	class BURGWAR_CORELIB_API Terrain
	{
		public:
			Terrain(Match& match, Map& map);
			Terrain(const Terrain&) = delete;
			~Terrain() = default;

			inline TerrainLayer& GetLayer(LayerIndex layerIndex);
			inline const TerrainLayer& GetLayer(LayerIndex layerIndex) const;
			inline LayerIndex GetLayerCount() const;
			inline const Map& GetMap() const;

			void Initialize();

			void Reset();

			void Update(float elapsedTime);

			Terrain& operator=(const Terrain&) = delete;

		private:
			Map& m_map;
			std::vector<std::unique_ptr<TerrainLayer>> m_layers;
	};
}

#include <CoreLib/Terrain.inl>

#endif
