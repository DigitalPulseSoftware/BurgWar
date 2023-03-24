// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_PLAYER_HPP
#define BURGWAR_CLIENTLIB_PLAYER_HPP

#include <CoreLib/EntityOwner.hpp>
#include <CoreLib/Export.hpp>
#include <CoreLib/LayerIndex.hpp>
#include <CoreLib/MatchClientSession.hpp>
#include <CoreLib/ScriptingEnvironment.hpp>
#include <CoreLib/Components/DestructionWatcherComponent.hpp>
#include <CoreLib/Components/HealthComponent.hpp>
#include <CoreLib/Components/WeaponWielderComponent.hpp>
#include <Nazara/Core/ObjectHandle.hpp>
#include <NazaraUtils/MovablePtr.hpp>
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

	class BURGWAR_CORELIB_API Player : public Nz::HandledObject<Player>
	{
		friend Match;

		public:
			Player(Match& match, MatchClientSession& session, std::size_t playerIndex, Nz::UInt8 localIndex, std::string name);
			Player(const Player&) = delete;
			Player(Player&&) noexcept = default;
			~Player();

			inline entt::handle GetControlledEntity() const;
			inline const PlayerInputData& GetInputs() const;
			inline LayerIndex GetLayerIndex() const;
			inline Nz::UInt8 GetLocalIndex() const;
			inline Match& GetMatch() const;
			inline const std::string& GetName() const;
			inline std::size_t GetPlayerIndex() const;
			inline MatchClientSession& GetSession();
			inline const MatchClientSession& GetSession() const;

			void HandleConsoleCommand(const std::string& str);

			inline bool IsAdmin() const;
			inline bool IsReady() const;

			void MoveToLayer(LayerIndex layerIndex);

			void PrintChatMessage(std::string message);

			void OnTick(bool lastTick);

			template<typename T> void SendPacket(const T& packet);
			void SetAdmin(bool isAdmin);

			std::string ToString() const;

			void UpdateControlledEntity(entt::handle entity, bool sendPacket = true, bool ignoreLayerUpdate = false);
			void UpdateLayerVisibility(LayerIndex layerIndex, bool isVisible);
			inline void UpdateInputs(const PlayerInputData& inputData);
			void UpdateName(std::string newName);

			Player& operator=(const Player&) = delete;
			Player& operator=(Player&&) = delete;

			static constexpr LayerIndex NoLayer = std::numeric_limits<LayerIndex>::max();
			static constexpr std::size_t NoWeapon = WeaponWielderComponent::NoWeapon;

		private:
			void OnDeath(entt::handle attacker);
			void SetReady();

			NazaraSlot(DestructionWatcherComponent, OnDestruction, m_onPlayerEntityDestruction);
			NazaraSlot(HealthComponent, OnDie, m_onPlayerEntityDie);
			NazaraSlot(WeaponWielderComponent, OnWeaponAdded, m_onWeaponAdded);
			NazaraSlot(WeaponWielderComponent, OnWeaponRemove, m_onWeaponRemove);

			std::optional<EntityOwner> m_playerEntity;
			std::optional<ScriptingEnvironment> m_scriptingEnvironment;
			LayerIndex m_layerIndex;
			std::size_t m_playerIndex;
			std::string m_name;
			Nz::Bitset<Nz::UInt64> m_visibleLayers;
			Nz::UInt8 m_localIndex;
			Match& m_match;
			MatchClientSession& m_session;
			PlayerInputData m_inputs;
			bool m_isAdmin;
			bool m_isReady;
			bool m_shouldSendWeapons;
	};
}

#include <CoreLib/Player.inl>

#endif
