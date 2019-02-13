// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_SHARED_TERRAINLAYER_HPP
#define BURGWAR_SHARED_TERRAINLAYER_HPP

#include <Common/Map.hpp>
#include <Shared/MapData.hpp>
#include <NDK/World.hpp>

namespace bw
{
	class BurgApp;
	class Match;

	class TerrainLayer
	{
		public:
			TerrainLayer(BurgApp& app, Match& match, const MapData::Layer& layerData, const Map::Layer& layerData2, float tileSize);
			TerrainLayer(const TerrainLayer&) = delete;
			TerrainLayer(TerrainLayer&&) = default;
			~TerrainLayer() = default;

			inline Ndk::World& GetWorld();

			void Update(float elapsedTime);

			TerrainLayer& operator=(const TerrainLayer&) = delete;
			TerrainLayer& operator=(TerrainLayer&&) = default;

		private:
			Ndk::EntityHandle m_camera;
			Ndk::World m_world;
	};
}

#include <Shared/TerrainLayer.inl>

#endif
