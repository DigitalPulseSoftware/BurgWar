// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_MAP_HPP
#define BURGWAR_CORELIB_MAP_HPP

#include <CoreLib/EntityId.hpp>
#include <CoreLib/Export.hpp>
#include <CoreLib/PropertyValues.hpp>
#include <CoreLib/LayerIndex.hpp>
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

	class BURGWAR_CORELIB_API Map
	{
		public:
			struct Asset;
			struct Entity;
			struct EntityIndices;
			struct Layer;
			struct PreserveUniqueId {};
			struct Script;

			inline Map();
			inline Map(MapInfo mapInfo);
			Map(const Map&) = default;
			Map(Map&&) noexcept = default;
			~Map() = default;

			template<typename... Args> Entity& AddEntity(LayerIndex layerIndex, Args&&... args);
			template<typename... Args> Entity& AddEntity(LayerIndex layerIndex, PreserveUniqueId, Args&&... args);
			template<typename... Args> Layer& AddLayer(Args&&... args);

			bool Compile(const std::filesystem::path& outputPath);

			inline Entity DropEntity(LayerIndex layerIndex, std::size_t entityIndex);
			inline Entity DropEntity(EntityId uniqueId);
			inline Layer DropLayer(LayerIndex layerIndex);

			template<typename... Args> Entity& EmplaceEntity(LayerIndex layerIndex, std::size_t entityIndex, Args&&... args);
			template<typename... Args> Layer& EmplaceLayer(LayerIndex layerIndex, Args&&... args);

			template<typename F> void ForeachEntity(F&& func);
			template<PropertyType P, typename F> void ForeachEntityProperty(F&& func);
			template<PropertyType P, typename F> void ForeachEntityPropertyValue(F&& func);

			inline EntityId GenerateUniqueId();

			inline std::vector<Asset>& GetAssets();
			inline const std::vector<Asset>& GetAssets() const;
			inline Entity& GetEntity(LayerIndex layerIndex, std::size_t entityIndex);
			inline const Entity& GetEntity(LayerIndex layerIndex, std::size_t entityIndex) const;
			inline Entity& GetEntity(EntityId uniqueId);
			inline const Entity& GetEntity(EntityId uniqueId) const;
			inline const EntityIndices& GetEntityIndices(EntityId uniqueId) const;
			inline std::size_t GetEntityCount(LayerIndex layerIndex) const;
			inline EntityId GetFreeUniqueId() const;
			inline Layer& GetLayer(LayerIndex layerIndex);
			inline const Layer& GetLayer(LayerIndex layerIndex) const;
			inline std::size_t GetLayerCount() const;
			inline std::vector<Layer>& GetLayers();
			inline const std::vector<Layer>& GetLayers() const;
			inline const MapInfo& GetMapInfo() const;
			inline std::vector<Script>& GetScripts();
			inline const std::vector<Script>& GetScripts() const;

			inline bool IsValid() const;

			inline Entity& MoveEntity(LayerIndex sourceLayerIndex, std::size_t sourceEntityIndex, LayerIndex targetLayerIndex, std::size_t targetEntityIndex);

			void RebuildEntityIndices();

			bool Save(const std::filesystem::path& mapFolderPath) const;

			void SwapEntities(LayerIndex layerIndex, std::size_t firstEntityIndex, std::size_t secondEntityIndex);
			void SwapLayers(LayerIndex firstLayerIndex, LayerIndex secondLayerIndex);

			Map& operator=(const Map&) = default;
			Map& operator=(Map&&) noexcept = default;

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
				EntityId uniqueId;
				Nz::Vector2f position;
				PropertyValueMap properties;
			};

			struct EntityIndices
			{
				LayerIndex layerIndex;
				std::size_t entityIndex;
			};

			struct Layer
			{
				Nz::Color backgroundColor = Nz::Color::Black;
				Nz::Vector2f positionAlignment = Nz::Vector2f::Unit();
				std::string name = "unnamed layer";
				std::vector<Entity> entities;
			};

			struct Script
			{
				std::string filepath;
				std::vector<Nz::UInt8> content;
			};

			static inline Map LoadFromBinary(const std::filesystem::path& mapFile);
			static inline Map LoadFromDirectory(const std::filesystem::path& mapDirectory);

			static nlohmann::json Serialize(const Map& map);
			static nlohmann::json SerializeEntity(const Entity& entity);
			static Map Unserialize(const nlohmann::json& mapInfo);
			static Entity UnserializeEntity(const nlohmann::json& entityInfo);

		private:
			bool CheckEntityIndices() const;
			void LoadFromBinaryInternal(const std::filesystem::path& mapFile);
			void LoadFromDirectoryInternal(const std::filesystem::path& mapFolder);
			inline void RegisterEntity(EntityId uniqueId, LayerIndex layerIndex, std::size_t entityIndex);
			void Sanitize();
			inline void UnregisterEntity(EntityId uniqueId);

			std::vector<Asset> m_assets;
			std::vector<Layer> m_layers;
			std::vector<Script> m_scripts;
			EntityId m_freeUniqueId;
			tsl::hopscotch_map<EntityId /*unique id*/, EntityIndices> m_entitiesByUniqueId;
			MapInfo m_mapInfo;
			bool m_isValid;
	};
}

#include <CoreLib/Map.inl>

#endif
