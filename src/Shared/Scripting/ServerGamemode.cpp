// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/Scripting/ServerGamemode.hpp>
#include <Nazara/Math/Vector2.hpp>
#include <NDK/Components.hpp>
#include <Shared/Match.hpp>
#include <Shared/Terrain.hpp>
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
						else if (value.is<Nz::Int64>()) //< Handle int before float
							entityProperties.emplace(std::move(propertyName), Nz::Int64(value.as<Nz::Int64>()));
						else if (value.is<float>())
							entityProperties.emplace(std::move(propertyName), float(value.as<float>()));
						else if (value.is<std::string>())
							entityProperties.emplace(std::move(propertyName), value.as<std::string>());
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
