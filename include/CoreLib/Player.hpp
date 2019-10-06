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
#include <CoreLib/ScriptingEnvironment.hpp>
#include <tsl/hopscotch_map.h>
#include <limits>
#include <optional>
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

			inline const Ndk::EntityHandle& GetControlledEntity() const;
			inline std::size_t GetLayerIndex() const;
			inline Match* GetMatch() const;
			inline const std::string& GetName() const;
			inline Nz::UInt8 GetPlayerIndex() const;
			inline MatchClientSession& GetSession();
			inline const MatchClientSession& GetSession() const;
			inline std::size_t GetWeaponCount() const;

			bool GiveWeapon(std::string weaponClass);

			void HandleConsoleCommand(const std::string& str);

			inline bool HasWeapon(const std::string& weaponClass) const;

			void RemoveWeapon(const std::string& weaponClass);

			inline bool IsInMatch() const;

			void Spawn();

			template<typename T> void SendPacket(const T& packet);

			void SelectWeapon(std::size_t weaponIndex);

			std::string ToString() const;

			void OnTick(bool lastTick);

			void UpdateControlledEntity(const Ndk::EntityHandle& entity);
			void UpdateInputs(const PlayerInputData& inputData);
			void UpdateInputs(std::size_t tickDelay, PlayerInputData inputData);

			Player& operator=(const Player&) = delete;
			Player& operator=(Player&&) = delete;

			static constexpr std::size_t NoWeapon = std::numeric_limits<std::size_t>::max();

		private:
			void OnDeath(const Ndk::EntityHandle& attacker);

			void UpdateLayer(std::size_t layerIndex);
			void UpdateMatch(Match* match);

			std::array<std::optional<PlayerInputData>, 10> m_inputBuffer;
			std::optional<ScriptingEnvironment> m_scriptingEnvironment;
			std::size_t m_layerIndex;
			std::size_t m_inputIndex;
			std::size_t m_activeWeaponIndex = NoWeapon;
			std::string m_name;
			std::vector<Ndk::EntityOwner> m_weapons;
			tsl::hopscotch_map<std::string /*weaponClass*/, std::size_t /*weaponIndex*/> m_weaponByName;
			Ndk::EntityOwner m_playerEntity;
			Nz::MovablePtr<Match> m_match;
			Nz::UInt8 m_playerIndex;
			MatchClientSession& m_session;
			bool m_shouldSendWeapons;
	};
}

#include <CoreLib/Player.inl>

#endif
