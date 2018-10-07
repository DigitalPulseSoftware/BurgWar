// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/Match.hpp>
#include <Client/Player.hpp>
#include <Client/Terrain.hpp>
#include <cassert>

namespace bw
{
	Match::Match(BurgApp& app, std::string matchName, std::size_t maxPlayerCount) :
	m_maxPlayerCount(maxPlayerCount),
	m_name(std::move(matchName)),
	m_application(app)
	{
		m_terrain = std::make_unique<Terrain>(app);
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

		m_players.push_back(player);
		player->UpdateMatch(this);

		player->UpdateLayer(0);
		player->CreateEntity(m_terrain->GetLayer(0).GetWorld());

		return true;
	}

	void Match::Update(float elapsedTime)
	{
		m_terrain->Update(elapsedTime);
	}
}
