// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/Gamemode.hpp>
#include <Nazara/Math/Vector2.hpp>
#include <NDK/Components.hpp>
#include <NDK/LuaAPI.hpp>
#include <Shared/Match.hpp>
#include <Shared/Terrain.hpp>
#include <cassert>
#include <iostream>

namespace bw
{
	Gamemode::Gamemode(Match& match, std::shared_ptr<SharedScriptingContext> scriptingContext, std::string gamemodeName, const std::filesystem::path& gamemodePath) :
	m_context(std::move(scriptingContext)),
	m_match(match)
	{
		auto Load = [&](const std::filesystem::path& filepath)
		{
			if (!std::filesystem::exists(filepath))
				return false;

			return m_context->Load(filepath);
		};

		sol::state& state = m_context->GetLuaState();

		m_gamemodeTable = state.create_table();
		m_gamemodeTable["Name"] = gamemodeName;
		state["GM"] = m_gamemodeTable;

		Load(gamemodePath / "shared.lua");
		//Load(gamemodePath / "cl_init.lua");
		Load(gamemodePath / "sv_init.lua");
	}

	void Gamemode::ExecuteCallback(const std::string& callbackName, const std::function<int(Nz::LuaState& /*state*/)>& argumentFunction)
	{
		sol::protected_function callback = m_gamemodeTable[callbackName];
		if (callback)
		{
			auto result = callback(m_gamemodeTable, 42);
			if (!result.valid())
				std::cerr << callbackName << " gamemode callback failed: " << result.abandon << std::endl;

			//TODO: Handle return
		}
	}

	void Gamemode::InitializeGamemode(Nz::LuaState& state)
	{
		state.PushFunction([&](Nz::LuaState& state) -> int
		{
			int index = 2;
			std::string entityType = state.Check<std::string>(&index);
			Nz::Vector2f spawnPos = state.Check<Nz::Vector2f>(&index);

			auto& entityStore = m_match.GetEntityStore();

			if (std::size_t entityIndex = entityStore.GetElementIndex(entityType); entityIndex != ServerEntityStore::InvalidIndex)
			{
				const Ndk::EntityHandle& entity = entityStore.InstantiateEntity(m_match.GetTerrain().GetLayer(0).GetWorld(), entityIndex);
				if (!entity)
				{
					state.Error("Failed to create \"" + entityType + "\"");
					return 0;
				}

				if (entity->HasComponent<Ndk::PhysicsComponent2D>())
					entity->GetComponent<Ndk::PhysicsComponent2D>().SetPosition(spawnPos);
				else
					entity->GetComponent<Ndk::NodeComponent>().SetPosition(spawnPos);

				//TODO: Return entity
				return 0;
			}
			else
			{
				state.Error("Entity type \"" + entityType + "\" doesn't exist");
				return 0;
			}
		});
		state.SetField("CreateEntity");
	}
}
