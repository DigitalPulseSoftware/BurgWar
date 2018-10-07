// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENT_PLAYER_HPP
#define BURGWAR_CLIENT_PLAYER_HPP

#include <NDK/EntityOwner.hpp>
#include <string>

namespace bw
{
	class Match;

	class Player
	{
		friend Match;

		public:
			Player(std::string name);
			Player(const Player&) = delete;
			~Player();

			const Ndk::EntityHandle& CreateEntity(Ndk::World& world);

			inline std::size_t GetLayerIndex() const;
			inline Match* GetMatch() const;

			inline bool IsInMatch() const;

			Player& operator=(const Player&) = delete;

		private:
			void UpdateLayer(std::size_t layerIndex);
			void UpdateMatch(Match* match);

			std::size_t m_layerIndex;
			std::string m_name;
			Ndk::EntityOwner m_playerEntity;
			Match* m_match;
	};
}

#include <Client/Player.inl>

#endif
