// Copyright (C) 2016 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in Prerequisites.hpp

#include <CoreLib/Map.hpp>
#include <CoreLib/Utils.hpp>
#include <Nazara/Core/ByteStream.hpp>
#include <Nazara/Core/ErrorFlags.hpp>
#include <Nazara/Core/File.hpp>
#include <nlohmann/json.hpp>
#include <iostream>
#include <stdexcept>

namespace Nz
{
	void to_json(nlohmann::json& j, const Nz::Color& color)
	{
		j = nlohmann::json{ {"r", color.r}, {"g", color.g}, {"b", color.b} };
	}

	void to_json(nlohmann::json& j, const Nz::Vector2f& vec)
	{
		j = nlohmann::json{ {"x", vec.x}, {"y", vec.y} };
	}

	void from_json(const nlohmann::json& j, Nz::Color& color)
	{
		j.at("r").get_to(color.r);
		j.at("g").get_to(color.g);
		j.at("b").get_to(color.b);
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
			layerInfo["backgroundColor"] = layerEntry.backgroundColor;
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

					std::visit([&](auto&& propertyValue)
					{
						using T = std::decay_t<decltype(propertyValue)>;
						constexpr bool IsArray = IsSameTpl_v<EntityPropertyArray, T>;
						using PropertyType = std::conditional_t<IsArray, IsSameTpl<EntityPropertyArray, T>::ContainedType, T>;

						if constexpr (std::is_same_v<PropertyType, bool> || 
						              std::is_same_v<PropertyType, float> || 
						              std::is_same_v<PropertyType, Nz::Int64> ||
						              std::is_same_v<PropertyType, std::string>)
						{
							if constexpr (IsArray)
							{
								auto elementArray = nlohmann::json::array();
								for (std::size_t i = 0; i < propertyValue.size(); ++i)
									elementArray.push_back(propertyValue[i]);

								propertiesObject[keyName] = std::move(elementArray);
							}
							else
								propertiesObject[keyName] = propertyValue;
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
		stream << m_mapInfo.name << m_mapInfo.author << m_mapInfo.description;

		// Map layers
		Nz::UInt16 layerCount = Nz::UInt16(m_layers.size());
		stream << layerCount;

		for (const Layer& layer : m_layers)
		{
			stream << layer.name;
			stream << layer.depth;
			stream << layer.backgroundColor;

			Nz::UInt16 entityCount = Nz::UInt16(layer.entities.size());
			stream << entityCount;

			for (const Entity& entity : layer.entities)
			{
				stream << entity.entityType;
				stream << entity.name;
				stream << entity.position.x << entity.position.y;
				stream << entity.rotation.ToDegrees();

				Nz::UInt8 propertyCount = Nz::UInt8(entity.properties.size());
				stream << propertyCount;

				for (const auto& [key, value] : entity.properties)
				{
					stream << key;

					Nz::UInt8 propertyType = Nz::UInt8(value.index());
					stream << propertyType;

					std::visit([&](auto&& propertyValue)
					{
						using T = std::decay_t<decltype(propertyValue)>;
						constexpr bool IsArray = IsSameTpl_v<EntityPropertyArray, T>;
						using PropertyType = std::conditional_t<IsArray, IsSameTpl<EntityPropertyArray, T>::ContainedType, T>;

						Nz::UInt8 isArray = (IsArray) ? 1 : 0;
						stream << isArray;

						auto Serialize = [&](const auto& value)
						{
							using T = std::decay_t<decltype(value)>;

							if constexpr (std::is_same_v<T, bool>)
							{
								Nz::UInt8 boolValue = (value) ? 1 : 0;
								stream << boolValue;
							}
							else if constexpr (std::is_same_v<T, float> || std::is_same_v<T, Nz::Int64> || std::is_same_v<T, std::string>)
							{
								stream << value;
							}
							else
								static_assert(AlwaysFalse<T>::value, "non-exhaustive visitor");
						};

						if constexpr (IsArray)
						{
							stream << Nz::UInt32(propertyValue.size());
							for (const auto& element : propertyValue)
								Serialize(element);
						}
						else
							Serialize(propertyValue);

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

	void Map::LoadFromBinaryInternal(const std::filesystem::path& mapFile)
	{
		Nz::File infoFile(mapFile.generic_u8string(), Nz::OpenMode_ReadOnly);
		if (!infoFile.IsOpen())
			throw std::runtime_error("Failed to open map file");

		Nz::ErrorFlags errFlags(Nz::ErrorFlag_ThrowException);

		Nz::ByteStream stream(&infoFile);
		stream.SetDataEndianness(Nz::Endianness_LittleEndian);

		std::array<char, 8> signature;
		if (stream.Read(signature.data(), signature.size()) != signature.size())
			throw std::runtime_error("Corrupted map file (or not a burger map file)");

		if (std::memcmp(signature.data(), "Burgrmap", signature.size()) != 0)
			throw std::runtime_error("Not a valid burger map file");

		Nz::UInt16 fileVersion;
		stream >> fileVersion;

		if (fileVersion != 0)
			throw std::runtime_error("Unhandled file version");

		// Map header
		stream >> m_mapInfo.name >> m_mapInfo.author >> m_mapInfo.description;

		Nz::UInt16 layerCount;
		stream >> layerCount;

		m_layers.clear();
		m_layers.resize(layerCount);

		std::size_t layerIndex = 0;
		for (Layer& layer : m_layers)
		{
			stream >> layer.name;
			stream >> layer.depth;
			stream >> layer.backgroundColor;

			Nz::UInt16 entityCount;
			stream >> entityCount;

			std::size_t entityIndex = 0;

			layer.entities.resize(entityCount);
			for (Entity& entity : layer.entities)
			{
				stream >> entity.entityType;
				stream >> entity.name;
				stream >> entity.position.x >> entity.position.y;

				float degRot;
				stream >> degRot;
				entity.rotation = Nz::DegreeAnglef::FromDegrees(degRot);

				Nz::UInt8 propertyCount;
				stream >> propertyCount;

				std::size_t loopCount = propertyCount;
				for (std::size_t i = 0; i < loopCount; ++i)
				{
					std::string propertyName;
					stream >> propertyName;

					Nz::UInt8 propertyType;
					stream >> propertyType;

					Nz::UInt8 isArrayInt;
					stream >> isArrayInt;

					bool isArray = (isArrayInt != 0);

					// Waiting for template lambda in C++20
					auto Unserialize = [&](auto dummyType)
					{
						using T = std::decay_t<decltype(dummyType)>;

						static_assert(std::is_same_v<T, bool> || std::is_same_v<T, float> || std::is_same_v<T, Nz::Int64> || std::is_same_v<T, std::string>);

						if (isArray)
						{
							Nz::UInt32 size;
							stream >> size;

							EntityPropertyArray<T> elements(size);
							for (T& element : elements)
								stream >> element;

							entity.properties.emplace(std::move(propertyName), std::move(elements));
						}
						else
						{
							T value;
							stream >> value;

							entity.properties.emplace(std::move(propertyName), std::move(value));
						}
					};

					switch (propertyType)
					{
						case 0: case 1: Unserialize(bool()); break;
						case 2: case 3: Unserialize(float()); break;
						case 4: case 5: Unserialize(Nz::Int64()); break;
						case 6: case 7: Unserialize(std::string()); break;

						default:
							throw std::runtime_error("Unexpected type index " + std::to_string(propertyType) + " for property " + propertyName + " for entity #" + std::to_string(entityIndex) + " in layer #" + std::to_string(layerIndex));
					}
				}

				entityIndex++;
			}

			layerIndex++;
		}

		m_isValid = true;
	}

	void Map::LoadFromTextInternal(const std::filesystem::path& mapFolder)
	{
		Nz::File infoFile((mapFolder / "info.json").generic_u8string(), Nz::OpenMode_ReadOnly);
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
			layer.backgroundColor = entry.value("backgroundColor", Nz::Color::Black);
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
					if (value.is_array())
					{
						std::size_t elementCount = value.size();
						if (elementCount == 0)
							continue; //< Ignore empty arrays

						if (value[0].is_boolean())
						{
							EntityPropertyArray<bool> elements(elementCount);
							for (std::size_t i = 0; i < elementCount; ++i)
								elements[i] = value[i];

							entity.properties.emplace(key, std::move(elements));
						}
						else if (value[0].is_number_integer())
						{
							EntityPropertyArray<Nz::Int64> elements(elementCount);
							for (std::size_t i = 0; i < elementCount; ++i)
								elements[i] = value[i];

							entity.properties.emplace(key, std::move(elements));
						}
						else if (value[0].is_number_float())
						{
							EntityPropertyArray<float> elements(elementCount);
							for (std::size_t i = 0; i < elementCount; ++i)
								elements[i] = value[i];

							entity.properties.emplace(key, std::move(elements));
						}
						else if (value[0].is_string())
						{
							EntityPropertyArray<std::string> elements(elementCount);
							for (std::size_t i = 0; i < elementCount; ++i)
								elements[i] = value[i];

							entity.properties.emplace(key, std::move(elements));
						}
						else
							std::cerr << "Invalid type for property " << key << ": (" << value.type_name() << ")" << std::endl;
					}
					else
					{
						if (value.is_boolean())
							entity.properties.emplace(key, bool(value));
						else if (value.is_number_integer())
							entity.properties.emplace(key, Nz::Int64(value));
						else if (value.is_number_float())
							entity.properties.emplace(key, float(value));
						else if (value.is_string())
							entity.properties.emplace(key, std::string(value));
						else
							std::cerr << "Invalid type for property " << key << ": (" << value.type_name() << ")" << std::endl;
					}
				}
			}
		}

		m_isValid = true;
	}

	void Map::SetupDefault()
	{
		Layer& layer = m_layers.emplace_back();
		layer.depth = 0.f;
		layer.name = "Default layer";
	}
}