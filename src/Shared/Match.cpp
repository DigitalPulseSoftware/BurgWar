// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/Match.hpp>
#include <Shared/MatchClientSession.hpp>
#include <Shared/Player.hpp>
#include <Shared/Terrain.hpp>
#include <Shared/Protocol/Packets.hpp>
#include <Shared/Systems/NetworkSyncSystem.hpp>
#include <NDK/Components/PhysicsComponent2D.hpp>
#include <cassert>

namespace bw
{
	Match::Match(BurgApp& app, std::string matchName, std::size_t maxPlayerCount) :
	m_sessions(*this),
	m_maxPlayerCount(maxPlayerCount),
	m_name(std::move(matchName))
	{
		MapData mapData;
		mapData.backgroundColor = Nz::Color::Cyan;
		mapData.tileSize = 64.f;

		auto& layer = mapData.layers.emplace_back();
		layer.width = 20;
		layer.height = 13;
		layer.tiles = {
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 2, 2,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1,
		};

		m_terrain = std::make_unique<Terrain>(app, std::move(mapData));

		m_scriptingContext = std::make_shared<SharedScriptingContext>(true);

		m_entityStore.emplace(m_scriptingContext);
		m_entityStore->Load("../../scripts/entities");

		m_entityStore->ForEachElement([&](const ScriptedEntity& entity)
		{
			if (entity.isNetworked)
				m_networkStringStore.RegisterString(entity.fullName);
		});

		m_weaponStore.emplace(app, m_scriptingContext);
		m_weaponStore->Load("../../scripts/weapons");

		m_weaponStore->ForEachElement([&](const ScriptedWeapon& weapon)
		{
			m_networkStringStore.RegisterString(weapon.fullName);
		});


		if (std::size_t entityIndex = m_entityStore->GetElementIndex("entity_box"); entityIndex != ServerEntityStore::InvalidIndex)
		{
			const Ndk::EntityHandle& box = m_entityStore->InstantiateEntity(m_terrain->GetLayer(0).GetWorld(), entityIndex);
			if (!box)
				return;

			box->GetComponent<Ndk::PhysicsComponent2D>().SetPosition({ 500.f, 100.f });
		}
	}

	Match::~Match() = default;

	void Match::Leave(Player* player)
	{
		assert(player->GetMatch() == this);

		auto it = std::find(m_players.begin(), m_players.end(), player);
		assert(it != m_players.end());

		m_players.erase(it);

		player->UpdateLayer(std::numeric_limits<std::size_t>::max());
		player->UpdateMatch(nullptr);
	}

	bool Match::Join(Player* player)
	{
		assert(!player->IsInMatch());

		if (m_players.size() >= m_maxPlayerCount)
			return false;

		m_players.emplace_back(player);
		player->UpdateMatch(this);

		player->UpdateLayer(0);
		player->CreateEntity(m_terrain->GetLayer(0).GetWorld());

		return true;
	}

	void Match::Update(float elapsedTime)
	{
		m_scriptingContext->Update();
		m_sessions.Poll();
		m_terrain->Update(elapsedTime);

		m_sessions.ForEachSession([&](MatchClientSession* session)
		{
			session->GetVisibility().UpdateLayer(0); //< HAAAAAX
			session->Update(elapsedTime);
		});
	}
}
