// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENT_PLAYER_HPP
#define BURGWAR_CLIENT_PLAYER_HPP

#include <Nazara/Core/ObjectHandle.hpp>
#include <Nazara/Core/MovablePtr.hpp>
#include <NDK/EntityOwner.hpp>
#include <Shared/MatchClientSession.hpp>
#include <string>

namespace bw
{
	class Match;
	class Player;

	using PlayerHandle = Nz::ObjectHandle<Player>;

	class Player : public Nz::HandledObject<Player>
	{
		friend Match;

		public:
			Player(MatchClientSession& session, std::string name);
			Player(const Player&) = delete;
			Player(Player&&) noexcept = default;
			~Player();

			const Ndk::EntityHandle& CreateEntity(Ndk::World& world);

			inline std::size_t GetLayerIndex() const;
			inline Match* GetMatch() const;

			inline bool IsInMatch() const;

			template<typename T> void SendPacket(const T& packet);

			void UpdateInput(bool isJumping, bool isMovingLeft, bool isMovingRight);

			Player& operator=(const Player&) = delete;
			Player& operator=(Player&&) noexcept = default;

		private:
			void UpdateLayer(std::size_t layerIndex);
			void UpdateMatch(Match* match);

			std::size_t m_layerIndex;
			std::string m_name;
			Ndk::EntityOwner m_playerEntity;
			Nz::MovablePtr<Match> m_match;
			MatchClientSession& m_session;
	};
}

#include <Shared/Player.inl>

#endif
