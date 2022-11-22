// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_TERRAINLAYER_HPP
#define BURGWAR_CORELIB_TERRAINLAYER_HPP

#include <CoreLib/Export.hpp>
#include <CoreLib/Map.hpp>
#include <CoreLib/SharedLayer.hpp>
#include <CoreLib/Systems/NetworkSyncSystem.hpp>

namespace bw
{
	class BurgApp;
	class Match;

	class BURGWAR_CORELIB_API TerrainLayer : public SharedLayer
	{
		friend class Terrain;

		public:
			TerrainLayer(Match& match, LayerIndex layerIndex, const Map::Layer& layerData);
			TerrainLayer(const TerrainLayer&) = delete;
			TerrainLayer(TerrainLayer&&) = delete;
			~TerrainLayer() = default;

			Match& GetMatch();
			inline NetworkSyncSystem& GetNetworkSyncSystem();

			void ResetEntities();

			TerrainLayer& operator=(const TerrainLayer&) = delete;
			TerrainLayer& operator=(TerrainLayer&&) = delete;

		private:
			void InitializeEntities();

			const Map::Layer& m_mapLayer;
	};
}

#include <CoreLib/TerrainLayer.inl>

#endif
