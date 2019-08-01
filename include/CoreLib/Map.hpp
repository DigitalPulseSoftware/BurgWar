// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_MAP_HPP
#define BURGWAR_CORELIB_MAP_HPP

#include <CoreLib/EntityProperties.hpp>
#include <Nazara/Core/Color.hpp>
#include <Nazara/Math/Angle.hpp>
#include <Nazara/Math/Vector2.hpp>
#include <nlohmann/json_fwd.hpp>
#include <tsl/hopscotch_map.h>
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
			struct Asset;
			struct Entity;
			struct Layer;

			inline Map();
			inline Map(MapInfo mapInfo);
			~Map() = default;

			nlohmann::json AsJson() const;

			bool Compile(const std::filesystem::path& outputPath);

			inline std::vector<Asset>& GetAssets();
			inline const std::vector<Asset>& GetAssets() const;
			inline Layer& GetLayer(std::size_t i);
			inline const Layer& GetLayer(std::size_t i) const;
			inline std::size_t GetLayerCount() const;
			inline const MapInfo& GetMapInfo() const;

			inline bool IsValid() const;

			bool Save(const std::filesystem::path& mapFolderPath) const;

			struct Asset
			{
				std::array<Nz::UInt8, 20> sha1Checksum;
				std::string filepath;
				Nz::UInt64 size;
			};

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
				Nz::Color backgroundColor = Nz::Color::Black;
				std::string name;
				float depth = 0.f;
				std::vector<Entity> entities;
			};

			static inline Map LoadFromBinary(const std::filesystem::path& mapFile);
			static inline Map LoadFromFolder(const std::filesystem::path& mapFolder);

		private:
			void LoadFromBinaryInternal(const std::filesystem::path& mapFile);
			void LoadFromTextInternal(const std::filesystem::path& mapFolder);
			void SetupDefault();

			MapInfo m_mapInfo;
			std::vector<Asset> m_assets;
			std::vector<Layer> m_layers;
			bool m_isValid;
	};
}

#include <CoreLib/Map.inl>

#endif
