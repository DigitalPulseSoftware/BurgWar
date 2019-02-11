// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENT_PLAYER_HPP
#define BURGWAR_CLIENT_PLAYER_HPP

#include <Nazara/Core/ObjectHandle.hpp>
#include <Nazara/Core/MovablePtr.hpp>
#include <NDK/EntityOwner.hpp>
#include <GameLibShared/MatchClientSession.hpp>
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
			Player(MatchClientSession& session, Nz::UInt8 playerIndex, std::string name);
			Player(const Player&) = delete;
			Player(Player&&) noexcept = default;
			~Player();

			const Ndk::EntityHandle& CreateEntity(Ndk::World& world);

			inline std::size_t GetLayerIndex() const;
			inline Match* GetMatch() const;

			inline bool IsInMatch() const;

			template<typename T> void SendPacket(const T& packet);

			void UpdateControlledEntity(const Ndk::EntityHandle& entity);
			void UpdateInputs(const InputData& inputData);

			Player& operator=(const Player&) = delete;
			Player& operator=(Player&&) noexcept = default;

		private:
			void UpdateLayer(std::size_t layerIndex);
			void UpdateMatch(Match* match);

			std::size_t m_layerIndex;
			std::string m_name;
			Ndk::EntityOwner m_playerEntity;
			Ndk::EntityOwner m_playerWeapon;
			Nz::MovablePtr<Match> m_match;
			Nz::UInt8 m_playerIndex;
			MatchClientSession& m_session;
	};
}

#include <GameLibShared/Player.inl>

#endif
