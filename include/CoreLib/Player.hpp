// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_PLAYER_HPP
#define BURGWAR_CLIENTLIB_PLAYER_HPP

#include <Nazara/Core/ObjectHandle.hpp>
#include <Nazara/Core/MovablePtr.hpp>
#include <NDK/EntityOwner.hpp>
#include <CoreLib/LayerIndex.hpp>
#include <CoreLib/MatchClientSession.hpp>
#include <CoreLib/ScriptingEnvironment.hpp>
#include <CoreLib/Components/HealthComponent.hpp>
#include <CoreLib/Components/WeaponWielderComponent.hpp>
#include <Thirdparty/tsl/hopscotch_map.h>
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
			Player(Match& match, MatchClientSession& session, std::size_t playerIndex, Nz::UInt8 localIndex, std::string name);
			Player(const Player&) = delete;
			Player(Player&&) noexcept = default;
			~Player();

			inline const Ndk::EntityHandle& GetControlledEntity() const;
			inline LayerIndex GetLayerIndex() const;
			inline Match& GetMatch() const;
			inline const std::string& GetName() const;
			inline Nz::UInt8 GetLocalIndex() const;
			inline std::size_t GetPlayerIndex() const;
			inline MatchClientSession& GetSession();
			inline const MatchClientSession& GetSession() const;
			std::size_t GetWeaponCount() const;

			bool GiveWeapon(std::string weaponClass);

			void HandleConsoleCommand(const std::string& str);

			bool HasWeapon(const std::string& weaponClass) const;

			inline bool IsAdmin() const;
			inline bool IsReady() const;

			void MoveToLayer(LayerIndex layerIndex);

			void PrintChatMessage(std::string message);

			void OnTick(bool lastTick);

			void RemoveWeapon(const std::string& weaponClass);

			void SelectWeapon(std::size_t weaponIndex);
			template<typename T> void SendPacket(const T& packet);
			void SetAdmin(bool isAdmin);

			std::string ToString() const;

			void UpdateControlledEntity(const Ndk::EntityHandle& entity, bool sendPacket = true, bool ignoreLayerUpdate = false);
			void UpdateLayerVisibility(LayerIndex layerIndex, bool isVisible);
			void UpdateInputs(const PlayerInputData& inputData);
			void UpdateName(std::string newName);

			Player& operator=(const Player&) = delete;
			Player& operator=(Player&&) = delete;

			static constexpr LayerIndex NoLayer = std::numeric_limits<LayerIndex>::max();
			static constexpr std::size_t NoWeapon = WeaponWielderComponent::NoWeapon;

		private:
			void OnDeath(const Ndk::EntityHandle& attacker);
			void SetReady();

			NazaraSlot(Ndk::Entity, OnEntityDestruction, m_onPlayerEntityDestruction);
			NazaraSlot(HealthComponent, OnDied, m_onPlayerEntityDied);
			NazaraSlot(WeaponWielderComponent, OnWeaponAdded, m_onWeaponAdded);
			NazaraSlot(WeaponWielderComponent, OnWeaponRemove, m_onWeaponRemove);

			struct Input
			{
				PlayerInputData data;
				Nz::UInt16 index;
			};

			std::optional<ScriptingEnvironment> m_scriptingEnvironment;
			LayerIndex m_layerIndex;
			std::size_t m_playerIndex;
			std::string m_name;
			Ndk::EntityOwner m_playerEntity;
			Nz::Bitset<Nz::UInt64> m_visibleLayers;
			Nz::UInt8 m_localIndex;
			Match& m_match;
			MatchClientSession& m_session;
			bool m_isAdmin;
			bool m_isReady;
			bool m_shouldSendWeapons;
	};
}

#include <CoreLib/Player.inl>

#endif
