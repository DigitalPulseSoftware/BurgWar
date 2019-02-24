// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/ServerGamemode.hpp>
#include <Nazara/Math/Vector2.hpp>
#include <NDK/Components.hpp>
#include <CoreLib/Match.hpp>
#include <CoreLib/Terrain.hpp>
#include <cassert>
#include <iostream>

namespace bw
{
	void ServerGamemode::InitializeGamemode()
	{
		auto& context = GetScriptingContext();

		auto Load = [&](const std::filesystem::path& filepath)
		{
			return context->Load(filepath);
		};

		sol::state& state = context->GetLuaState();
		state["GM"] = GetGamemodeTable();

		const std::filesystem::path& gamemodePath = GetGamemodePath();
		Load(gamemodePath / "shared.lua");
		Load(gamemodePath / "sv_init.lua");

		state["GM"] = nullptr;

		sol::table& gamemodeTable = GetGamemodeTable();
		gamemodeTable["CreateEntity"] = [&](const sol::table& gmTable, const std::string& entityType, const Nz::Vector2f& spawnPos, const sol::object& properties)
		{
			auto& entityStore = m_match.GetEntityStore();

			if (std::size_t entityIndex = entityStore.GetElementIndex(entityType); entityIndex != ServerEntityStore::InvalidIndex)
			{
				EntityProperties entityProperties;
				if (properties)
				{
					sol::table propertiesTable = properties;
					for (auto&& [key, value] : propertiesTable)
					{
						std::string propertyName = key.as<std::string>();

						if (value.is<bool>())
							entityProperties.emplace(std::move(propertyName), value.as<bool>());
						else if (value.is<Nz::Int64>()) //< Handle int before float to prevent implicit conversion
							entityProperties.emplace(std::move(propertyName), value.as<Nz::Int64>());
						else if (value.is<float>())
							entityProperties.emplace(std::move(propertyName), value.as<float>());
						else if (value.is<std::string>())
							entityProperties.emplace(std::move(propertyName), value.as<std::string>());
						else if (value.is<sol::table>())
						{
							sol::table content = value.as<sol::table>();
							std::size_t elementCount = content.size();
							if (elementCount == 0)
								continue; //< ignore empty array
							
							// Get first element
							sol::object firstElement = content[1];
							assert(firstElement);

							auto HandleDataArray = [&](auto dummyType)
							{
								using T = std::decay_t<decltype(dummyType)>;

								EntityPropertyArray<T> container(elementCount);
								for (std::size_t i = 0; i < elementCount; ++i)
									container[i] = content[i];

								entityProperties.emplace(std::move(propertyName), std::move(container));
							};

							if (firstElement.is<bool>())
								HandleDataArray(bool());
							else if (firstElement.is<Nz::Int64>()) //< Handle int before float to prevent implicit conversion
								HandleDataArray(Nz::Int64());
							else if (firstElement.is<float>())
								HandleDataArray(float());
							else if (firstElement.is<std::string>())
								HandleDataArray(std::string());
							else
								throw std::runtime_error("Unknown property type for " + propertyName);
						}
						else
							throw std::runtime_error("Unknown property type for " + propertyName);
					}
				}

				const Ndk::EntityHandle& entity = entityStore.InstantiateEntity(m_match.GetTerrain().GetLayer(0).GetWorld(), entityIndex, spawnPos, 0.f, entityProperties);
				if (!entity)
					throw std::runtime_error("Failed to create \"" + entityType + "\"");

				return entity;
			}
			else
				throw std::runtime_error("Entity type \"" + entityType + "\" doesn't exist");
		};
	}
}
