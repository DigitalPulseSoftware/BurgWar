// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_MAP_HPP
#define BURGWAR_CORELIB_MAP_HPP

#include <CoreLib/EntityProperties.hpp>
#include <CoreLib/LayerIndex.hpp>
#include <Nazara/Core/Color.hpp>
#include <Nazara/Math/Angle.hpp>
#include <Nazara/Math/Vector2.hpp>
#include <nlohmann/json_fwd.hpp>
#include <Thirdparty/tsl/hopscotch_map.h>
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
			struct EntityIndices;
			struct Layer;

			inline Map();
			inline Map(MapInfo mapInfo);
			~Map() = default;

			template<typename... Args> Entity& AddEntity(std::size_t layerIndex, Args&&... args);
			template<typename... Args> Layer& AddLayer(Args&&... args);
			nlohmann::json AsJson() const;

			bool Compile(const std::filesystem::path& outputPath);

			inline Entity DropEntity(std::size_t layerIndex, std::size_t entityIndex);
			inline Layer DropLayer(std::size_t layerIndex);

			template<typename... Args> Entity& EmplaceEntity(std::size_t layerIndex, std::size_t entityIndex, Args&&... args);
			template<typename... Args> Layer& EmplaceLayer(std::size_t layerIndex, Args&&... args);

			template<typename F> void ForeachEntity(F&& func);

			inline Nz::Int64 GenerateUniqueId();

			inline std::vector<Asset>& GetAssets();
			inline const std::vector<Asset>& GetAssets() const;
			inline Entity& GetEntity(std::size_t layerIndex, std::size_t entityIndex);
			inline const Entity& GetEntity(std::size_t layerIndex, std::size_t entityIndex) const;
			inline Entity& GetEntity(Nz::Int64 uniqueId);
			inline const Entity& GetEntity(Nz::Int64 uniqueId) const;
			inline const EntityIndices& GetEntityIndices(Nz::Int64 uniqueId) const;
			inline std::size_t GetEntityCount(std::size_t layerIndex) const;
			inline Nz::Int64 GetFreeUniqueId() const;
			inline Layer& GetLayer(std::size_t layerIndex);
			inline const Layer& GetLayer(std::size_t layerIndex) const;
			inline std::size_t GetLayerCount() const;
			inline const MapInfo& GetMapInfo() const;

			inline bool IsValid() const;

			Entity& MoveEntity(std::size_t sourceLayerIndex, std::size_t sourceEntityIndex, std::size_t targetLayerIndex);

			bool Save(const std::filesystem::path& mapFolderPath) const;

			struct Asset
			{
				static constexpr std::size_t ChecksumSize = 20;

				std::array<Nz::UInt8, ChecksumSize> sha1Checksum;
				std::string filepath;
				Nz::UInt64 size;
			};

			struct Entity
			{
				std::string entityType;
				std::string name;
				Nz::DegreeAnglef rotation;
				Nz::Int64 uniqueId;
				Nz::Vector2f position;
				EntityProperties properties;
			};

			struct EntityIndices
			{
				std::size_t layerIndex;
				std::size_t entityIndex;
			};

			struct Layer
			{
				Nz::Color backgroundColor = Nz::Color::Black;
				Nz::Vector2f positionAlignment = Nz::Vector2f::Unit();
				std::string name = "unnamed layer";
				std::vector<Entity> entities;
			};

			static inline Map LoadFromBinary(const std::filesystem::path& mapFile);
			static inline Map LoadFromFolder(const std::filesystem::path& mapFolder);

		private:
			void LoadFromBinaryInternal(const std::filesystem::path& mapFile);
			void LoadFromTextInternal(const std::filesystem::path& mapFolder);
			inline void RegisterEntity(Nz::Int64 uniqueId, std::size_t layerIndex, std::size_t entityIndex);
			void Sanitize();
			void SetupDefault();
			inline void UnregisterEntity(Nz::Int64 uniqueId);

			std::vector<Asset> m_assets;
			std::vector<Layer> m_layers;
			Nz::Int64 m_freeUniqueId;
			tsl::hopscotch_map<Nz::Int64 /*unique id*/, EntityIndices> m_entitiesByUniqueId;
			MapInfo m_mapInfo;
			bool m_isValid;
	};
}

#include <CoreLib/Map.inl>

#endif
