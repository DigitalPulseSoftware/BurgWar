// Copyright (C) 2016 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in Prerequisites.hpp

#include <CoreLib/Map.hpp>
#include <CoreLib/Protocol/CompressedInteger.hpp>
#include <CoreLib/Utils.hpp>
#include <Nazara/Core/Bitset.hpp>
#include <Nazara/Core/ByteStream.hpp>
#include <Nazara/Core/ErrorFlags.hpp>
#include <Nazara/Core/File.hpp>
#include <Nazara/Math/Rect.hpp>
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
			assetInfo["size"] = assetEntry.size;

			assetArray.emplace_back(std::move(assetInfo));
		}
		mapInfo["assets"] = std::move(assetArray);

		auto layerArray = nlohmann::json::array();
		for (auto&& layerEntry : m_layers)
		{
			nlohmann::json layerInfo;
			layerInfo["backgroundColor"] = layerEntry.backgroundColor;
			layerInfo["name"] = layerEntry.name;

			auto entityArray = nlohmann::json::array();
			for (auto&& entityEntry : layerEntry.entities)
			{
				nlohmann::json entityInfo;
				entityInfo["entityType"] = entityEntry.entityType;
				entityInfo["name"] = entityEntry.name;
				entityInfo["position"] = entityEntry.position;
				entityInfo["rotation"] = entityEntry.rotation.ToDegrees();
				entityInfo["uniqueId"] = entityEntry.uniqueId;

				auto propertiesObject = nlohmann::json::object();
				for (auto&& propertyPair : entityEntry.properties)
				{
					const std::string& keyName = propertyPair.first;

					auto [internalType, isArray] = ExtractPropertyType(propertyPair.second);

					auto propertyData = nlohmann::json::object();
					propertyData["type"] = ToString(internalType);

					std::visit([&](auto&& propertyValue)
					{
						using T = std::decay_t<decltype(propertyValue)>;
						constexpr bool IsArray = IsSameTpl_v<EntityPropertyArray, T>;

						if constexpr (IsArray)
						{
							auto elementArray = nlohmann::json::array();
							for (std::size_t i = 0; i < propertyValue.size(); ++i)
								elementArray.push_back(propertyValue[i]);

							propertyData["isArray"] = true;
							propertyData["value"] = std::move(elementArray);
						}
						else
							propertyData["value"] = propertyValue;

					}, propertyPair.second);

					propertiesObject[keyName] = std::move(propertyData);
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
			stream << layer.backgroundColor;

			Nz::UInt16 entityCount = Nz::UInt16(layer.entities.size());
			stream << entityCount;

			for (const Entity& entity : layer.entities)
			{
				stream << entity.entityType;
				stream << entity.name;
				stream << entity.position.x << entity.position.y;
				stream << entity.rotation.ToDegrees();

				CompressedSigned<Nz::Int64> compressedUniqueId;
				stream << compressedUniqueId;

				Nz::UInt8 propertyCount = Nz::UInt8(entity.properties.size());
				stream << propertyCount;

				for (const auto& [key, value] : entity.properties)
				{
					stream << key;

					auto [internalType, isArray] = ExtractPropertyType(value);

					Nz::UInt8 propertyType = Nz::UInt8(internalType);
					stream << propertyType;

					stream << Nz::UInt8((isArray) ? 1 : 0);

					std::visit([&](auto&& propertyValue)
					{
						using T = std::decay_t<decltype(propertyValue)>;
						constexpr bool IsArray = IsSameTpl_v<EntityPropertyArray, T>;

						auto Serialize = [&](const auto& value)
						{
							using T = std::decay_t<decltype(value)>;

							if constexpr (std::is_same_v<T, bool>)
							{
								Nz::UInt8 boolValue = (value) ? 1 : 0;
								stream << boolValue;
							}
							else
								stream << value;
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
		stream << empty;

		// Scripts (TODO)
		/*std::vector<std::string> scripts;

		for (const auto& scriptPath : std::filesystem::recursive_directory_iterator(m_mapPath / "scripts"))
			scripts.emplace_back(scriptPath.path().generic_u8string());

		Nz::UInt32 scriptCount = Nz::UInt32(scripts.size());*/

		// Assets
		Nz::UInt32 assetCount = Nz::UInt32(m_assets.size());
		stream << assetCount;

		for (const Asset& asset : m_assets)
		{
			stream << asset.filepath;
			stream << asset.size;
			stream.Write(asset.sha1Checksum.data(), asset.sha1Checksum.size());
		}

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

		for (Layer& layer : m_layers)
		{
			stream >> layer.name;
			stream >> layer.backgroundColor;

			Nz::UInt16 entityCount;
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

				CompressedSigned<Nz::Int64> compressedUniqueId;
				stream >> compressedUniqueId;
				entity.uniqueId = compressedUniqueId;

				Nz::UInt8 propertyCount;
				stream >> propertyCount;

				std::size_t loopCount = propertyCount;
				for (std::size_t i = 0; i < loopCount; ++i)
				{
					std::string propertyName;
					stream >> propertyName;

					Nz::UInt8 propertyTypeInt;
					stream >> propertyTypeInt;

					PropertyInternalType propertyType = static_cast<PropertyInternalType>(propertyTypeInt);

					Nz::UInt8 isArrayInt;
					stream >> isArrayInt;

					bool isArray = (isArrayInt != 0);

					// Waiting for template lambda in C++20
					auto Unserialize = [&](auto dummyType)
					{
						using T = std::decay_t<decltype(dummyType)>;

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
						case PropertyInternalType::Bool: Unserialize(bool()); break;
						case PropertyInternalType::Float: Unserialize(float()); break;
						case PropertyInternalType::Float2: Unserialize(Nz::Vector2f()); break;
						case PropertyInternalType::Float3: Unserialize(Nz::Vector3f()); break;
						case PropertyInternalType::Float4: Unserialize(Nz::Vector4f()); break;
						case PropertyInternalType::Integer: Unserialize(Nz::Int64()); break;
						case PropertyInternalType::Integer2: Unserialize(Nz::Vector2i64()); break;
						case PropertyInternalType::Integer3: Unserialize(Nz::Vector3i64()); break;
						case PropertyInternalType::Integer4: Unserialize(Nz::Vector4i64()); break;
						case PropertyInternalType::String: Unserialize(std::string()); break;
					}
				}
			}
		}

		Nz::UInt32 scriptCount;
		stream >> scriptCount;

		// TODO

		Nz::UInt32 assetCount;
		stream >> assetCount;

		m_assets.clear();
		m_assets.resize(assetCount);

		for (Asset& asset : m_assets)
		{
			stream >> asset.filepath;
			stream >> asset.size;
			stream.Read(asset.sha1Checksum.data(), asset.sha1Checksum.size());
		}

		Sanitize();
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
			asset.size = entry.value("size", Nz::UInt64(0));
		}

		m_layers.clear();
		for (auto&& entry : json["layers"])
		{
			Layer& layer = m_layers.emplace_back();
			layer.backgroundColor = entry.value("backgroundColor", Nz::Color::Black);
			layer.name = entry.value("name", "");

			for (auto&& entityInfo : entry["entities"])
			{
				Entity& entity = layer.entities.emplace_back();
				entity.entityType = entityInfo.at("entityType");
				entity.name = entityInfo.value("name", "");
				entity.position = entityInfo.at("position");
				entity.rotation = Nz::DegreeAnglef(float(entityInfo.at("rotation")));
				entity.uniqueId = entityInfo.value("uniqueId", NoEntity);
				
				for (auto&& [propertyName, propertyData] : entityInfo["properties"].items())
				{
					bool isArray = propertyData.value<bool>("isArray", false);
					PropertyInternalType propertyType = ParsePropertyInternalType(propertyData.at("type"));
					auto&& value = propertyData.at("value");

					// Waiting for template lambda in C++20
					auto Unserialize = [&, propertyName = propertyName](auto dummyType)
					{
						using T = std::decay_t<decltype(dummyType)>;

						if (isArray)
						{
							if (!value.is_array())
								throw std::runtime_error("Expected array");

							std::size_t elementCount = value.size();
							if (elementCount == 0)
								return; //< Ignore empty arrays

							EntityPropertyArray<T> elements(elementCount);
							for (std::size_t i = 0; i < elementCount; ++i)
								elements[i] = value[i];

							entity.properties.emplace(std::move(propertyName), std::move(elements));
						}
						else
						{
							T propertyValue = value;
							entity.properties.emplace(std::move(propertyName), std::move(propertyValue));
						}
					};

					switch (propertyType)
					{
						case PropertyInternalType::Bool: Unserialize(bool()); break;
						case PropertyInternalType::Float: Unserialize(float()); break;
						case PropertyInternalType::Float2: Unserialize(Nz::Vector2f()); break;
						case PropertyInternalType::Float3: Unserialize(Nz::Vector3f()); break;
						case PropertyInternalType::Float4: Unserialize(Nz::Vector4f()); break;
						case PropertyInternalType::Integer: Unserialize(Nz::Int64()); break;
						case PropertyInternalType::Integer2: Unserialize(Nz::Vector2i64()); break;
						case PropertyInternalType::Integer3: Unserialize(Nz::Vector3i64()); break;
						case PropertyInternalType::Integer4: Unserialize(Nz::Vector4i64()); break;
						case PropertyInternalType::String: Unserialize(std::string()); break;
					}
				}
			}
		}

		Sanitize();
		m_isValid = true;
	}

	void Map::Sanitize()
	{
		// Ensures every entity gets an unique id
		Nz::Int64 biggestId = 0;
		for (const auto& layer : m_layers)
		{
			for (const auto& entity : layer.entities)
				biggestId = std::max(biggestId, entity.uniqueId);
		}

		std::size_t layerIndex = 0;
		for (auto& layer : m_layers)
		{
			std::size_t entityIndex = 0;
			for (auto& entity : layer.entities)
			{
				if (entity.uniqueId <= 0)
					entity.uniqueId = ++biggestId;

				RegisterEntity(entity.uniqueId, layerIndex, entityIndex);

				entityIndex++;
			}

			layerIndex++;
		}

		m_freeUniqueId = ++biggestId;
	}

	void Map::SetupDefault()
	{
		Layer& layer = m_layers.emplace_back();
		layer.name = "Layer #1";
	}
}
