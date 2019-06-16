// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_PLAYER_HPP
#define BURGWAR_CLIENTLIB_PLAYER_HPP

#include <Nazara/Core/ObjectHandle.hpp>
#include <Nazara/Core/MovablePtr.hpp>
#include <NDK/EntityOwner.hpp>
#include <CoreLib/MatchClientSession.hpp>
#include <tsl/hopscotch_map.h>
#include <limits>
#include <string>
#include <vector>

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

			inline std::size_t GetLayerIndex() const;
			inline Match* GetMatch() const;
			inline const std::string& GetName() const;
			inline const MatchClientSession& GetSession() const;

			bool GiveWeapon(std::string weaponClass);

			inline bool HasWeapon(const std::string& weaponClass);

			inline bool IsInMatch() const;

			void Spawn();

			template<typename T> void SendPacket(const T& packet);

			std::string ToString() const;

			void OnTick();

			void UpdateControlledEntity(const Ndk::EntityHandle& entity);
			void UpdateInputs(const InputData& inputData);
			void UpdateInputs(std::size_t tickDelay, InputData inputData);

			Player& operator=(const Player&) = delete;
			Player& operator=(Player&&) noexcept = default;

			static constexpr std::size_t NoWeapon = std::numeric_limits<std::size_t>::max();

		private:
			void UpdateLayer(std::size_t layerIndex);
			void UpdateMatch(Match* match);

			std::array<std::optional<InputData>, 10> m_inputBuffer;
			std::size_t m_layerIndex;
			std::size_t m_inputIndex;
			std::size_t m_weaponIndex = NoWeapon;
			std::string m_name;
			tsl::hopscotch_map<std::string /*weaponClass*/, Ndk::EntityOwner /*weapon*/> m_weapons;
			Ndk::EntityOwner m_playerEntity;
			Nz::MovablePtr<Match> m_match;
			Nz::UInt8 m_playerIndex;
			MatchClientSession& m_session;
	};
}

#include <CoreLib/Player.inl>

#endif
