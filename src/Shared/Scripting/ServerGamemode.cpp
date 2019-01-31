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
		gamemodeTable["CreateEntity"] = [&](const sol::table& gmTable, const std::string& entityType, const Nz::Vector2f& spawnPos)
		{
			auto& entityStore = m_match.GetEntityStore();

			if (std::size_t entityIndex = entityStore.GetElementIndex(entityType); entityIndex != ServerEntityStore::InvalidIndex)
			{
				const Ndk::EntityHandle& entity = entityStore.InstantiateEntity(m_match.GetTerrain().GetLayer(0).GetWorld(), entityIndex, { { "size", 3.f * rand() / float(RAND_MAX) + 0.5f } });
				if (!entity)
					throw std::runtime_error("Failed to create \"" + entityType + "\"");

				if (entity->HasComponent<Ndk::PhysicsComponent2D>())
					entity->GetComponent<Ndk::PhysicsComponent2D>().SetPosition(spawnPos);
				else
					entity->GetComponent<Ndk::NodeComponent>().SetPosition(spawnPos);

				return entity;
			}
			else
				throw std::runtime_error("Entity type \"" + entityType + "\" doesn't exist");
		};
	}
}
