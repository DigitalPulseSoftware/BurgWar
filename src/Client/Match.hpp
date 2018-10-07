// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENT_MATCH_HPP
#define BURGWAR_CLIENT_MATCH_HPP

#include <memory>
#include <string>
#include <vector>

namespace bw
{
	class BurgApp;
	class Player;
	class Terrain;

	class Match
	{
		public:
			Match(BurgApp& app, std::string matchName, std::size_t maxPlayerCount);
			Match(const Match&) = delete;
			~Match();

			void Leave(Player* player);
			bool Join(Player* player);

			void Update(float elapsedTime);

			Terrain& operator=(const Terrain&) = delete;

		private:
			std::size_t m_maxPlayerCount;
			std::string m_name;
			std::unique_ptr<Terrain> m_terrain;
			std::vector<Player*> m_players;
			BurgApp& m_application;
	};
}

#include <Client/Match.inl>

#endif
