// Copyright (C) 2016 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in Prerequisites.hpp

#include <CoreLib/Map.hpp>
#include <CoreLib/Protocol/CompressedInteger.hpp>
#include <CoreLib/Version.hpp>
#include <CoreLib/Utils.hpp>
#include <Nazara/Utils/Bitset.hpp>
#include <Nazara/Core/ByteStream.hpp>
#include <Nazara/Core/ErrorFlags.hpp>
#include <Nazara/Core/File.hpp>
#include <Nazara/Math/Rect.hpp>
#include <fmt/format.h>
#include <nlohmann/json.hpp>
#include <stdexcept>

namespace Nz
{
	void from_json(const nlohmann::json& j, Nz::Color& color)
	{
		j.at("r").get_to(color.r);
		j.at("g").get_to(color.g);
		j.at("b").get_to(color.b);
	}

	void to_json(nlohmann::json& j, const Nz::Color& color)
	{
		j = nlohmann::json{ {"r", color.r}, {"g", color.g}, {"b", color.b} };
	}

	template<typename T>
	void from_json(const nlohmann::json& j, Nz::DegreeAngle<T>& angle)
	{
		angle = Nz::DegreeAngle<T>(T(j));
	}

	template<typename T>
	void to_json(nlohmann::json& j, const Nz::DegreeAngle<T>& angle)
	{
		j = angle.ToDegrees();
	}

	template<typename T>
	void from_json(const nlohmann::json& j, Nz::Rect<T>& rect)
	{
		j.at("x").get_to(rect.x);
		j.at("y").get_to(rect.y);
		j.at("width").get_to(rect.width);
		j.at("height").get_to(rect.height);
	}

	template<typename T>
	void to_json(nlohmann::json& j, const Nz::Rect<T>& rect)
	{
		j = nlohmann::json{ {"x", rect.x}, {"y", rect.y}, {"width", rect.width}, {"height", rect.height} };
	}

	template<typename T>
	void from_json(const nlohmann::json& j, Nz::Vector2<T>& vec)
	{
		j.at("x").get_to(vec.x);
		j.at("y").get_to(vec.y);
	}

	template<typename T>
	void to_json(nlohmann::json& j, const Nz::Vector2<T>& vec)
	{
		j = nlohmann::json{ {"x", vec.x}, {"y", vec.y} };
	}

	template<typename T>
	void from_json(const nlohmann::json& j, Nz::Vector3<T>& vec)
	{
		j.at("x").get_to(vec.x);
		j.at("y").get_to(vec.y);
		j.at("z").get_to(vec.z);
	}

	template<typename T>
	void to_json(nlohmann::json& j, const Nz::Vector3<T>& vec)
	{
		j = nlohmann::json{ {"x", vec.x}, {"y", vec.y}, {"z", vec.z} };
	}

	template<typename T>
	void from_json(const nlohmann::json& j, Nz::Vector4<T>& vec)
	{
		j.at("x").get_to(vec.x);
		j.at("y").get_to(vec.y);
		j.at("z").get_to(vec.z);
		j.at("w").get_to(vec.w);
	}

	template<typename T>
	void to_json(nlohmann::json& j, const Nz::Vector4<T>& vec)
	{
		j = nlohmann::json{ {"x", vec.x}, {"y", vec.y}, {"z", vec.z}, {"w", vec.w} };
	}
}

namespace bw
{
	constexpr Nz::UInt16 MapFileVersion = 1;

	bool Map::Compile(const std::filesystem::path& outputPath)
	{
		Nz::File infoFile(outputPath.generic_u8string(), Nz::OpenMode::WriteOnly | Nz::OpenMode::Truncate);
		if (!infoFile.IsOpen())
			return false;

		Nz::ByteStream stream(&infoFile);
		stream.SetDataEndianness(Nz::Endianness::LittleEndian);

		stream.Write("Burgrmap", 8);
		stream << MapFileVersion;

		// Map header
		stream << m_mapInfo.name << m_mapInfo.author << m_mapInfo.description;

		// Game version
		stream << GameVersion;

		// Map layers
		CompressedUnsigned<Nz::UInt16> layerCount(Nz::UInt16(m_layers.size()));
		stream << layerCount;

		for (const Layer& layer : m_layers)
		{
			stream << layer.name;
			stream << layer.backgroundColor;

			CompressedUnsigned<Nz::UInt16> entityCount(Nz::UInt16(layer.entities.size()));
			stream << entityCount;

			for (const Entity& entity : layer.entities)
			{
				stream << entity.entityType;
				stream << entity.name;
				stream << entity.position.x << entity.position.y;
				stream << entity.rotation.ToDegrees();

				CompressedSigned<EntityId> compressedUniqueId(entity.uniqueId);
				stream << compressedUniqueId;

				CompressedUnsigned<Nz::UInt16> propertyCount(Nz::UInt16(entity.properties.size()));
				stream << propertyCount;

				for (const auto& [key, value] : entity.properties)
				{
					stream << key;

					auto [P, isArray] = ExtractPropertyType(value);

					Nz::UInt8 propertyType = Nz::UInt8(P);
					stream << propertyType;

					stream << isArray;

					std::visit([&](auto&& propertyValue)
					{
						using T = std::decay_t<decltype(propertyValue)>;
						using TypeExtractor = PropertyTypeExtractor<T>;
						constexpr bool IsArray = TypeExtractor::IsArray;

						if constexpr (IsArray)
						{
							CompressedUnsigned<Nz::UInt32> arraySize(Nz::UInt32(propertyValue.size()));

							stream << arraySize;
							for (const auto& element : propertyValue)
								stream << element;
						}
						else
							stream << propertyValue.value;

					}, value);
				}
			}
		}

		// Scripts
		CompressedUnsigned<Nz::UInt32> scriptCount(Nz::UInt32(m_scripts.size()));
		stream << scriptCount;

		for (const auto& script : m_scripts)
		{
			stream << script.filepath;
			CompressedUnsigned<Nz::UInt64> scriptSize(Nz::UInt64(script.content.size()));
			stream << scriptSize;
			stream.Write(script.content.data(), script.content.size());
		}

		// Assets
		CompressedUnsigned<Nz::UInt32> assetCount(Nz::UInt32(m_assets.size()));
		stream << assetCount;

		for (const Asset& asset : m_assets)
		{
			stream << asset.filepath;
			stream << asset.size;
			stream.Write(asset.sha1Checksum.data(), asset.sha1Checksum.size());
		}

		return true;
	}

	void Map::RebuildEntityIndices()
	{
		m_entitiesByUniqueId.clear();

		LayerIndex layerIndex = 0;
		for (auto& layer : m_layers)
		{
			std::size_t entityIndex = 0;
			for (auto& entity : layer.entities)
			{
				if (entity.uniqueId > 0)
					RegisterEntity(entity.uniqueId, layerIndex, entityIndex);

				entityIndex++;
			}

			layerIndex++;
		}

		assert(CheckEntityIndices());
	}

	bool Map::Save(const std::filesystem::path& mapFolderPath) const
	{
		assert(IsValid());

		std::string content = Serialize(*this).dump(1, '\t');

		Nz::File infoFile((mapFolderPath / "info.json").generic_u8string(), Nz::OpenMode::WriteOnly | Nz::OpenMode::Truncate);
		if (!infoFile.IsOpen())
			return false;

		if (infoFile.Write(content.data(), content.size()) != content.size())
			return false;

		return true;
	}

	nlohmann::json Map::Serialize(const Map& map)
	{
		assert(map.IsValid());

		const MapInfo& mapInfo = map.GetMapInfo();

		nlohmann::json mapJson;
		mapJson["name"] = mapInfo.name;
		mapJson["author"] = mapInfo.author;
		mapJson["description"] = mapInfo.description;
		mapJson["gameVersion"] = fmt::format("{}.{}.{}", GameMajorVersion, GameMinorVersion, GamePatchVersion);

		auto assetArray = nlohmann::json::array();
		for (const auto& mapAsset : map.GetAssets())
		{
			nlohmann::json assetInfo;
			assetInfo["filePath"] = mapAsset.filepath;
			assetInfo["checksum"] = mapAsset.sha1Checksum;
			assetInfo["size"] = mapAsset.size;

			assetArray.emplace_back(std::move(assetInfo));
		}
		mapJson["assets"] = std::move(assetArray);

		auto layerArray = nlohmann::json::array();
		for (const auto& mapLayer : map.GetLayers())
		{
			nlohmann::json layerInfo;
			layerInfo["backgroundColor"] = mapLayer.backgroundColor;
			layerInfo["name"] = mapLayer.name;

			auto entityArray = nlohmann::json::array();
			for (auto&& entityEntry : mapLayer.entities)
				entityArray.emplace_back(SerializeEntity(entityEntry));

			layerInfo["entities"] = std::move(entityArray);

			layerArray.emplace_back(std::move(layerInfo));
		}
		mapJson["layers"] = std::move(layerArray);

		return mapJson;
	}

	nlohmann::json Map::SerializeEntity(const Entity& entity)
	{
		nlohmann::json entityInfo;
		entityInfo["entityType"] = entity.entityType;
		entityInfo["name"] = entity.name;
		entityInfo["position"] = entity.position;
		entityInfo["rotation"] = entity.rotation.ToDegrees();
		entityInfo["uniqueId"] = entity.uniqueId;

		auto propertiesObject = nlohmann::json::object();
		for (auto&& propertyPair : entity.properties)
		{
			const std::string& keyName = propertyPair.first;

			auto [P, isArray] = ExtractPropertyType(propertyPair.second);

			auto propertyData = nlohmann::json::object();
			propertyData["type"] = ToString(P);

			std::visit([&](auto&& propertyValue)
			{
				using T = std::decay_t<decltype(propertyValue)>;
				using TypeExtractor = PropertyTypeExtractor<T>;
				constexpr bool IsArray = TypeExtractor::IsArray;

				if constexpr (IsArray)
				{
					auto elementArray = nlohmann::json::array();
					for (std::size_t i = 0; i < propertyValue.size(); ++i)
						elementArray.push_back(propertyValue[i]);

					propertyData["isArray"] = true;
					propertyData["value"] = std::move(elementArray);
				}
				else
					propertyData["value"] = propertyValue.value;

			}, propertyPair.second);

			propertiesObject[keyName] = std::move(propertyData);
		}
		entityInfo["properties"] = std::move(propertiesObject);

		return entityInfo;
	}

	Map Map::Unserialize(const nlohmann::json& mapJson)
	{
		MapInfo mapInfo;

		mapInfo.author = mapJson.value("author", "unknown");
		mapInfo.description = mapJson.value("description", "");
		mapInfo.name = mapJson.at("name");

		Map map(std::move(mapInfo));

		auto& assets = map.GetAssets();
		for (auto&& entry : mapJson["assets"])
		{
			Asset& asset = assets.emplace_back();
			asset.filepath = entry.at("filePath");
			asset.sha1Checksum = entry.at("checksum");
			asset.size = entry.value("size", Nz::UInt64(0));
		}

		auto& layers = map.GetLayers();
		for (auto&& entry : mapJson["layers"])
		{
			Layer& layer = layers.emplace_back();
			layer.backgroundColor = entry.value("backgroundColor", Nz::Color::Black);
			layer.name = entry.value("name", "");

			for (auto&& entityInfo : entry["entities"])
				layer.entities.emplace_back(UnserializeEntity(entityInfo));
		}

		map.RebuildEntityIndices();
		map.Sanitize();

		return map;
	}

	auto Map::UnserializeEntity(const nlohmann::json& entityInfo) -> Entity
	{
		Entity entity;
		entity.entityType = entityInfo.at("entityType");
		entity.name = entityInfo.value("name", "");
		entity.position = entityInfo.at("position");
		entity.rotation = Nz::DegreeAnglef(entityInfo.value("rotation", 0.f));
		entity.uniqueId = entityInfo.value("uniqueId", InvalidEntityId);

		for (auto&& [propertyName, propertyData] : entityInfo["properties"].items())
		{
			bool isArray = propertyData.value<bool>("isArray", false);
			std::string type = propertyData.at("type");
			PropertyType propertyType = ParsePropertyType(type);
			auto&& value = propertyData.at("value");

			// Waiting for template lambda in C++20
			auto Unserialize = [&, propertyName = propertyName](auto dummyType)
			{
				using T = std::decay_t<decltype(dummyType)>;

				static constexpr PropertyType Property = T::Property;
				using UnderlyingType = PropertyUnderlyingType_t<Property>;

				if (isArray)
				{
					if (!value.is_array())
						throw std::runtime_error("Expected array");

					std::size_t elementCount = value.size();
					if (elementCount == 0)
						return; //< Ignore empty arrays

					PropertyArrayValue<Property> elements(elementCount);
					for (std::size_t i = 0; i < elementCount; ++i)
						elements[i] = value[i];

					entity.properties.emplace(std::move(propertyName), std::move(elements));
				}
				else
				{
					UnderlyingType extractedValue = value;
					PropertySingleValue<Property> propertyValue(std::move(extractedValue));
					entity.properties.emplace(std::move(propertyName), std::move(propertyValue));
				}
			};

			switch (propertyType)
			{
#define BURGWAR_PROPERTYTYPE(V, T, IT) case PropertyType:: T: Unserialize(PropertyTag<PropertyType:: T>{}); break;

#include <CoreLib/PropertyTypeList.hpp>
			}
		}

		return entity;
	}

	bool Map::CheckEntityIndices() const
	{
		for (auto&& [uniqueId, indices] : m_entitiesByUniqueId)
		{
			if (indices.layerIndex >= m_layers.size())
				return false;

			const auto& layer = m_layers[indices.layerIndex];
			if (indices.entityIndex >= layer.entities.size())
				return false;

			if (layer.entities[indices.entityIndex].uniqueId != uniqueId)
				return false;
		}

		return true;
	}

	void Map::LoadFromBinaryInternal(const std::filesystem::path& mapFile)
	{
		Nz::File infoFile(mapFile.generic_u8string(), Nz::OpenMode::ReadOnly);
		if (!infoFile.IsOpen())
			throw std::runtime_error("failed to open map file");

		Nz::ErrorFlags errFlags(Nz::ErrorMode::ThrowException);

		Nz::ByteStream stream(&infoFile);
		stream.SetDataEndianness(Nz::Endianness::LittleEndian);

		std::array<char, 8> signature;
		if (stream.Read(signature.data(), signature.size()) != signature.size())
			throw std::runtime_error("corrupted map file (or not a burger map file)");

		if (std::memcmp(signature.data(), "Burgrmap", signature.size()) != 0)
			throw std::runtime_error("not a valid burger map file");

		Nz::UInt16 fileVersion;
		stream >> fileVersion;

		if (fileVersion > MapFileVersion)
			throw std::runtime_error("unhandled file version (more recent than game)");

		// Map header
		stream >> m_mapInfo.name >> m_mapInfo.author >> m_mapInfo.description;

		// For debugging purpose
		if (fileVersion >= 1)
		{
			Nz::UInt32 gameVersion;
			stream >> gameVersion;
			NazaraUnused(gameVersion);
		}

		CompressedUnsigned<Nz::UInt16> layerCount;
		stream >> layerCount;

		m_layers.clear();
		m_layers.resize(layerCount);

		for (Layer& layer : m_layers)
		{
			stream >> layer.name;
			stream >> layer.backgroundColor;

			CompressedUnsigned<Nz::UInt16> entityCount;
			stream >> entityCount;

			layer.entities.resize(entityCount);
			for (Entity& entity : layer.entities)
			{
				stream >> entity.entityType;
				stream >> entity.name;
				stream >> entity.position.x >> entity.position.y;

				float degRot;
				stream >> degRot;
				entity.rotation = Nz::DegreeAnglef::FromDegrees(degRot);

				CompressedSigned<EntityId> compressedUniqueId;
				stream >> compressedUniqueId;
				entity.uniqueId = compressedUniqueId;

				CompressedUnsigned<Nz::UInt16> propertyCount;
				stream >> propertyCount;

				std::size_t loopCount = propertyCount;
				for (std::size_t i = 0; i < loopCount; ++i)
				{
					std::string propertyName;
					stream >> propertyName;

					Nz::UInt8 propertyTypeInt;
					stream >> propertyTypeInt;

					PropertyType propertyType = static_cast<PropertyType>(propertyTypeInt);

					bool isArray;
					stream >> isArray;

					// Waiting for template lambda in C++20
					auto Unserialize = [&](auto dummyType)
					{
						using T = std::decay_t<decltype(dummyType)>;

						static constexpr PropertyType Property = T::Property;

						if (isArray)
						{
							CompressedUnsigned<Nz::UInt32> size;
							stream >> size;

							PropertyArrayValue<Property> elements(size);
							for (auto& element : elements)
								stream >> element;

							entity.properties.emplace(std::move(propertyName), std::move(elements));
						}
						else
						{
							PropertySingleValue<Property> value;
							stream >> value.value;

							entity.properties.emplace(std::move(propertyName), std::move(value));
						}
					};

					switch (propertyType)
					{
#define BURGWAR_PROPERTYTYPE(V, T, IT) case PropertyType:: T: Unserialize(PropertyTag<PropertyType:: T>{}); break;

#include <CoreLib/PropertyTypeList.hpp>
					}
				}
			}
		}

		// Scripts
		CompressedUnsigned<Nz::UInt32> scriptCount;
		stream >> scriptCount;

		m_scripts.clear();
		m_scripts.resize(scriptCount);

		for (Script& script : m_scripts)
		{
			stream >> script.filepath;

			CompressedUnsigned<Nz::UInt64> scriptSize;
			stream >> scriptSize;

			script.content.resize(scriptSize);
			stream.Read(script.content.data(), script.content.size());
		}

		// Assets
		CompressedUnsigned<Nz::UInt32> assetCount;
		stream >> assetCount;

		m_assets.clear();
		m_assets.resize(assetCount);

		for (Asset& asset : m_assets)
		{
			stream >> asset.filepath;
			stream >> asset.size;
			stream.Read(asset.sha1Checksum.data(), asset.sha1Checksum.size());
		}

		RebuildEntityIndices();
		Sanitize();

		m_isValid = true;
	}

	void Map::LoadFromDirectoryInternal(const std::filesystem::path& mapFolder)
	{
		std::vector<Nz::UInt8> content;

		Nz::File infoFile((mapFolder / "info.json").generic_u8string(), Nz::OpenMode::ReadOnly);
		if (!infoFile.IsOpen())
			throw std::runtime_error("failed to open info.json file");

		content.resize(infoFile.GetSize());
		if (infoFile.Read(content.data(), content.size()) != content.size())
			throw std::runtime_error("failed to read info.json file");

		nlohmann::json json = nlohmann::json::parse(content.begin(), content.end());

		Map map = Unserialize(json);

		// Load map scripts
		std::filesystem::path mapScriptDir = mapFolder / "scripts";
		if (std::filesystem::is_directory(mapScriptDir))
		{
			for (std::filesystem::path filepath : std::filesystem::recursive_directory_iterator(mapScriptDir))
			{
				if (!std::filesystem::is_regular_file(filepath))
					continue;

				std::string filepathStr = filepath.generic_u8string();

				Nz::File file(filepath.generic_u8string());
				if (!file.Open(Nz::OpenMode::ReadOnly))
					throw std::runtime_error("failed to open map script " + filepathStr);

				content.resize(file.GetSize());
				if (file.Read(content.data(), content.size()) != content.size())
					throw std::runtime_error("failed to read map script " + filepathStr);

				std::filesystem::path relativePath = std::filesystem::relative(filepath, mapScriptDir);

				map.m_scripts.push_back({
					relativePath.generic_u8string(),
					std::move(content)
				});
			}
		}

		operator=(std::move(map));
	}

	void Map::Sanitize()
	{
		assert(CheckEntityIndices());

		// Ensures every entity gets an unique id
		EntityId biggestId = 0;
		for (const auto& layer : m_layers)
		{
			for (const auto& entity : layer.entities)
				biggestId = std::max(biggestId, entity.uniqueId);
		}

		// Reuse free unique ids
		EntityId lowestId = 0;
		auto GetUniqueId = [&]() -> EntityId
		{
			// We can't make a set out of free unique ids up until the biggest id, as this would cause problems if the biggest id is huge
			// so we do it using an optimized for loop (keeping the lower bound from one call to the other)

			if (lowestId < biggestId)
			{
				for (++lowestId; lowestId < biggestId; ++lowestId)
				{
					if (m_entitiesByUniqueId.find(lowestId) == m_entitiesByUniqueId.end())
						return lowestId;
				}
			}

			return ++biggestId;
		};

		LayerIndex layerIndex = 0;
		for (auto& layer : m_layers)
		{
			std::size_t entityIndex = 0;
			for (auto& entity : layer.entities)
			{
				if (entity.uniqueId <= 0)
				{
					entity.uniqueId = GetUniqueId();
					RegisterEntity(entity.uniqueId, layerIndex, entityIndex);
				}

				entityIndex++;
			}

			layerIndex++;
		}

		m_freeUniqueId = ++biggestId;

		assert(CheckEntityIndices());
	}
}
