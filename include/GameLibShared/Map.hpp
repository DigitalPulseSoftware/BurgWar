// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_SHARED_MAP_HPP
#define BURGWAR_SHARED_MAP_HPP

#include <GameLibShared/EntityProperties.hpp>
#include <Nazara/Math/Angle.hpp>
#include <Nazara/Math/Vector2.hpp>
#include <nlohmann/json_fwd.hpp>
#include <hopscotch/hopscotch_map.h>
#include <array>
#include <filesystem>
#include <vector>

namespace bw
{
	struct MapInfo
	{
		std::string author;
		std::string description;
		std::string name;
	};

	class Map
	{
		public:
			struct Entity;
			struct Layer;

			inline Map();
			inline Map(MapInfo mapInfo);
			inline Map(const std::filesystem::path& folder);
			~Map() = default;

			nlohmann::json AsJson() const;

			bool Compile(const std::filesystem::path& outputPath);
			inline Layer& GetLayer(std::size_t i);
			inline const Layer& GetLayer(std::size_t i) const;
			inline std::size_t GetLayerCount() const;
			inline const MapInfo& GetMapInfo() const;

			inline bool IsValid() const;

			bool Save(const std::filesystem::path& mapFolderPath) const;

			struct Entity
			{
				std::string entityType;
				std::string name;
				Nz::DegreeAnglef rotation;
				Nz::Vector2f position;
				EntityProperties properties;
			};

			struct Layer
			{
				std::string name;
				float depth;
				std::vector<Entity> entities;
			};

		private:
			void Load(const std::filesystem::path& path);
			void SetupDefault();

			struct Asset
			{
				std::array<Nz::UInt8, 20> sha1Checksum;
				std::string filepath;
			};

			MapInfo m_mapInfo;
			std::filesystem::path m_mapPath;
			std::vector<Asset> m_assets;
			std::vector<Layer> m_layers;
			bool m_isValid;
	};
}

#include <GameLibShared/Map.inl>

#endif
