// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_SERVER_CLIENTSESSION_HPP
#define BURGWAR_SERVER_CLIENTSESSION_HPP

#include <Nazara/Core/HandledObject.hpp>
#include <Nazara/Core/ObjectHandle.hpp>
#include <CoreLib/PlayerCommandStore.hpp>
#include <CoreLib/SessionBridge.hpp>
#include <CoreLib/Protocol/Packets.hpp>
#include <memory>
#include <vector>

namespace bw
{
	class Match;
	class MatchClientVisibility;
	class Player;
	class PlayerCommandStore;

	using MatchClientSessionHandle = Nz::ObjectHandle<MatchClientSession>;
	using PlayerHandle = Nz::ObjectHandle<Player>;

	// Match client session
	class MatchClientSession final : public Nz::HandledObject<MatchClientSession>
	{
		friend PlayerCommandStore;

		public:
			MatchClientSession(Match& match, std::size_t sessionId, PlayerCommandStore& commandStore, std::shared_ptr<SessionBridge> bridge);
			MatchClientSession(const MatchClientSession&) = delete;
			MatchClientSession(MatchClientSession&&) = delete;
			~MatchClientSession();

			void Disconnect();

			template<typename F> void ForEachPlayer(F&& func);

			inline Nz::UInt32 GetPing() const;
			inline std::size_t GetSessionId() const;
			inline MatchClientVisibility& GetVisibility();
			inline const MatchClientVisibility& GetVisibility() const;

			void HandleIncomingPacket(Nz::NetPacket& packet);

			template<typename T> void SendPacket(const T& packet);

			void Update(float elapsedTime);

			MatchClientSession& operator=(const MatchClientSession&) = delete;
			MatchClientSession& operator=(MatchClientSession&&) = delete;

		private:
			void HandleIncomingPacket(const Packets::Auth& packet);
			void HandleIncomingPacket(const Packets::DownloadClientScriptRequest& packet);
			void HandleIncomingPacket(Packets::PlayerChat&& packet);
			void HandleIncomingPacket(const Packets::PlayerConsoleCommand& packet);
			void HandleIncomingPacket(const Packets::PlayersInput& packet);
			void HandleIncomingPacket(const Packets::PlayerSelectWeapon& packet);
			void HandleIncomingPacket(const Packets::Ready& packet);
			void HandleIncomingPacket(const Packets::ScriptPacket& packet);
			void HandleIncomingPacket(Packets::UpdatePlayerName&& packet);
			void UpdatePeerInfo(const SessionBridge::SessionInfo& sessionInfo);

			Match& m_match;
			PlayerCommandStore& m_commandStore;
			std::size_t m_sessionId;
			std::shared_ptr<SessionBridge> m_bridge;
			std::unique_ptr<MatchClientVisibility> m_visibility;
			std::vector<PlayerHandle> m_players;
			Nz::UInt32 m_ping;
			float m_peerInfoUpdateCounter;
	};
}

#include <CoreLib/MatchClientSession.inl>

#endif
