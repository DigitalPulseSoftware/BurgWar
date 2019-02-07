// Copyright (C) 2016 Jérôme Leclercq
// This file is part of the "Nazara Development Kit Qt Layer"
// For conditions of distribution and use, see copyright notice in Prerequisites.hpp

#include <MapEditor/EntityTypeRegistry.hpp>
#include <Nazara/Core/File.hpp>
#include <nlohmann/json.hpp>
#include <iostream>

namespace bw
{
	void EntityTypeRegistry::Reload()
	{
		m_entityTypeByName.clear();
		m_entityTypes.clear();

		for (auto&& entry : std::filesystem::recursive_directory_iterator(m_entityTypeFolder))
		{
			if (!entry.is_regular_file())
				continue;

			const std::filesystem::path& path = entry.path();
			if (path.extension() != ".json")
				continue;

			try
			{
				Nz::File typeFile(path.generic_u8string(), Nz::OpenMode_ReadOnly);
				if (!typeFile.IsOpen())
					throw std::runtime_error("Failed to open file");

				std::vector<Nz::UInt8> content(typeFile.GetSize());
				if (typeFile.Read(content.data(), content.size()) != content.size())
					throw std::runtime_error("Failed to read file");

				nlohmann::json json = nlohmann::json::parse(content.begin(), content.end());
				if (!json.is_object())
					throw std::runtime_error("Invalid JSon (must be an object)");

				EntityType entityType;
				entityType.name = path.stem().generic_u8string();
				entityType.description = json.value("description", "");

				nlohmann::json properties = json["properties"];
				if (!properties.is_null())
				{
					if (!properties.is_array())
						throw std::runtime_error("Invalid properties type (must be an array)");

					std::size_t counter = 1;
					for (auto&& propertyEntry : properties)
					{
						if (!propertyEntry.is_object())
							throw std::runtime_error("Invalid property type #" + std::to_string(counter) + " (must be an object)");

						Property property;

						property.keyName = propertyEntry["key"];
						property.visualName = propertyEntry.value("name", property.keyName);

						std::string typeStr = propertyEntry["type"];

						property.type = ParsePropertyType(typeStr);

						entityType.properties.emplace_back(std::move(property));

						counter++;
					}
				}

				std::size_t typeIndex = m_entityTypes.size();
				m_entityTypeByName.emplace(entityType.name, typeIndex);
				m_entityTypes.emplace_back(std::move(entityType));
			}
			catch (const std::exception& e)
			{
				std::cerr << "Failed to load entity type at " << path << ": " << e.what() << std::endl;
				continue;
			}
		}
	}

	EntityTypeRegistry* EntityTypeRegistry::s_instance = nullptr;
}
