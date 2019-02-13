// Copyright (C) 2016 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in Prerequisites.hpp

#include <CoreLib/Map.hpp>
#include <CoreLib/Utils.hpp>
#include <Nazara/Core/ByteStream.hpp>
#include <Nazara/Core/File.hpp>
#include <nlohmann/json.hpp>
#include <iostream>
#include <stdexcept>

namespace Nz
{
	void to_json(nlohmann::json& j, const Nz::Vector2f& vec)
	{
		j = nlohmann::json{ {"x", vec.x}, {"y", vec.y} };
	}

	void from_json(const nlohmann::json& j, Nz::Vector2f& vec)
	{
		j.at("x").get_to(vec.x);
		j.at("y").get_to(vec.y);
	}
}

namespace bw
{
	nlohmann::json Map::AsJson() const
	{
		assert(IsValid());

		nlohmann::json mapInfo;
		mapInfo["name"] = m_mapInfo.name;
		mapInfo["author"] = m_mapInfo.author;
		mapInfo["description"] = m_mapInfo.description;

		auto assetArray = nlohmann::json::array();
		for (auto&& assetEntry : m_assets)
		{
			nlohmann::json assetInfo;
			assetInfo["filePath"] = assetEntry.filepath;
			assetInfo["checksum"] = assetEntry.sha1Checksum;

			assetArray.emplace_back(std::move(assetInfo));
		}
		mapInfo["assets"] = std::move(assetArray);

		auto layerArray = nlohmann::json::array();
		for (auto&& layerEntry : m_layers)
		{
			nlohmann::json layerInfo;
			layerInfo["name"] = layerEntry.name;
			layerInfo["depth"] = layerEntry.depth;

			auto entityArray = nlohmann::json::array();
			for (auto&& entityEntry : layerEntry.entities)
			{
				nlohmann::json entityInfo;
				entityInfo["entityType"] = entityEntry.entityType;
				entityInfo["name"] = entityEntry.name;
				entityInfo["position"] = entityEntry.position;
				entityInfo["rotation"] = entityEntry.rotation.ToDegrees();

				auto propertiesObject = nlohmann::json::object();
				for (auto&& propertyPair : entityEntry.properties)
				{
					const std::string& keyName = propertyPair.first;

					std::visit([&](auto&& value)
					{
						using T = std::decay_t<decltype(value)>;

						if constexpr (std::is_same_v<T, bool> || std::is_same_v<T, float> || std::is_same_v<T, Nz::Int64> || std::is_same_v<T, std::string>)
						{
							propertiesObject[keyName] = value;
						}
						else if constexpr (std::is_same_v<T, std::monostate>)
						{
							// Ignore
						}
						else
							static_assert(AlwaysFalse<T>::value, "non-exhaustive visitor");

					}, propertyPair.second);
				}
				entityInfo["properties"] = std::move(propertiesObject);

				entityArray.emplace_back(std::move(entityInfo));
			}
			layerInfo["entities"] = std::move(entityArray);

			layerArray.emplace_back(std::move(layerInfo));
		}
		mapInfo["layers"] = std::move(layerArray);

		return mapInfo;
	}

	bool Map::Compile(const std::filesystem::path& outputPath)
	{
		Nz::File infoFile(outputPath.generic_u8string(), Nz::OpenMode_WriteOnly | Nz::OpenMode_Truncate);
		if (!infoFile.IsOpen())
			return false;

		constexpr Nz::UInt16 FileVersion = 0;

		Nz::ByteStream stream(&infoFile);
		stream.SetDataEndianness(Nz::Endianness_LittleEndian);

		stream.Write("Burgrmap", 8);
		stream << FileVersion;

		// Map header
		stream.Write(m_mapInfo.name.data(), m_mapInfo.name.size() + 1);
		stream.Write(m_mapInfo.author.data(), m_mapInfo.author.size() + 1);
		stream.Write(m_mapInfo.description.data(), m_mapInfo.description.size() + 1);

		// Map layers
		Nz::UInt16 layerCount = Nz::UInt16(m_layers.size());
		stream << layerCount;

		for (const Layer& layer : m_layers)
		{
			stream.Write(layer.name.data(), layer.name.size() + 1);
			stream << layer.depth;

			Nz::UInt16 entityCount = Nz::UInt16(layer.entities.size());
			stream << entityCount;

			for (const Entity& entity : layer.entities)
			{
				stream.Write(entity.entityType.data(), entity.entityType.size() + 1);
				stream.Write(entity.name.data(), entity.name.size() + 1);
				stream << entity.position.x << entity.position.y;
				stream << entity.rotation.ToDegrees();

				Nz::UInt8 propertyCount = Nz::UInt8(entity.properties.size());
				for (const auto& [key, value] : entity.properties)
				{
					stream.Write(key.data(), key.size() + 1);

					Nz::UInt8 propertyType = Nz::UInt8(value.index());
					stream << propertyType;

					std::visit([&](auto&& value)
					{
						using T = std::decay_t<decltype(value)>;

						if constexpr (std::is_same_v<T, bool>)
						{
							Nz::UInt8 boolValue((value) ? 1 : 0);
							stream << boolValue;
						}
						else if constexpr (std::is_same_v<T, float> || std::is_same_v<T, Nz::Int64>)
						{
							stream << value;
						}
						else if constexpr (std::is_same_v<T, std::string>)
						{
							Nz::UInt32 size = Nz::UInt32(value.size());
							stream << size;
							stream.Write(value.data(), value.size());
						}
						else if constexpr (std::is_same_v<T, std::monostate>)
						{
							// Nothing to write
						}
						else
							static_assert(AlwaysFalse<T>::value, "non-exhaustive visitor");

					}, value);
				}
			}
		}

		Nz::UInt32 empty = 0;
		stream << empty << empty;

		// Scripts (TODO)
		/*std::vector<std::string> scripts;

		for (const auto& scriptPath : std::filesystem::recursive_directory_iterator(m_mapPath / "scripts"))
			scripts.emplace_back(scriptPath.path().generic_u8string());

		Nz::UInt32 scriptCount = Nz::UInt32(scripts.size());*/

		// Assets (TODO)

		return true;
	}

	bool Map::Save(const std::filesystem::path& mapFolderPath) const
	{
		assert(IsValid());

		std::string content = AsJson().dump(1, '\t');

		Nz::File infoFile((mapFolderPath / "info.json").generic_u8string(), Nz::OpenMode_WriteOnly | Nz::OpenMode_Truncate);
		if (!infoFile.IsOpen())
			return false;

		if (infoFile.Write(content.data(), content.size()) != content.size())
			return false;

		return true;
	}

	void Map::Load(const std::filesystem::path& path)
	{
		Nz::File infoFile((path / "info.json").generic_u8string(), Nz::OpenMode_ReadOnly);
		if (!infoFile.IsOpen())
			throw std::runtime_error("Failed to open info.json file");

		std::vector<Nz::UInt8> content(infoFile.GetSize());
		if (infoFile.Read(content.data(), content.size()) != content.size())
			throw std::runtime_error("Failed to read info.json file");

		nlohmann::json json = nlohmann::json::parse(content.begin(), content.end());
		m_mapInfo.author = json.value("author", "unknown");
		m_mapInfo.description = json.value("description", "");
		m_mapInfo.name = json.at("name");

		m_assets.clear();
		for (auto&& entry : json["assets"])
		{
			Asset& asset = m_assets.emplace_back();
			asset.filepath = entry.at("filePath");
			asset.sha1Checksum = entry.at("checksum");
		}

		m_layers.clear();
		for (auto&& entry : json["layers"])
		{
			Layer& layer = m_layers.emplace_back();
			layer.depth = entry.value("depth", 0.f);
			layer.name = entry.value("name", "");

			for (auto&& entityInfo : entry["entities"])
			{
				Entity& entity = layer.entities.emplace_back();
				entity.entityType = entityInfo.at("entityType");
				entity.name = entityInfo.value("name", "");
				entity.position = entityInfo.at("position");
				entity.rotation = Nz::DegreeAnglef(float(entityInfo.at("rotation")));

				for (auto&& [key, value] : entityInfo["properties"].items())
				{
					if (value.is_boolean())
						entity.properties[key] = bool(value);
					else if (value.is_number_integer())
						entity.properties[key] = Nz::Int64(value);
					else if (value.is_number_float())
						entity.properties[key] = float(value);
					else if (value.is_string())
						entity.properties[key] = std::string(value);
					else
						std::cerr << "Invalid type for property " << key << " << (" << value.type_name() << ")" << std::endl;
				}
			}
		}
	}

	void Map::SetupDefault()
	{
		Layer& layer = m_layers.emplace_back();
		layer.depth = 0.f;
		layer.name = "Default layer";
	}
}