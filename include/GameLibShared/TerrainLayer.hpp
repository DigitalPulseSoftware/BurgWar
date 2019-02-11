// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_SHARED_TERRAINLAYER_HPP
#define BURGWAR_SHARED_TERRAINLAYER_HPP

#include <GameLibShared/MapData.hpp>
#include <NDK/World.hpp>

namespace bw
{
	class BurgApp;

	class TerrainLayer
	{
		public:
			TerrainLayer(BurgApp& app, const MapData::Layer& layerData, float tileSize);
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

#include <GameLibShared/TerrainLayer.inl>

#endif
