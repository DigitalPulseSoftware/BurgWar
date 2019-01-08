// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_SERVER_CLIENTSESSION_HPP
#define BURGWAR_SERVER_CLIENTSESSION_HPP

#include <Shared/MatchClientVisibility.hpp>
#include <Shared/PlayerCommandStore.hpp>
#include <Shared/SessionBridge.hpp>
#include <Shared/Protocol/Packets.hpp>
#include <memory>
#include <vector>

namespace bw
{
	class Match;
	class Player;
	class PlayerCommandStore;

	// Match client session
	class MatchClientSession final
	{
		friend PlayerCommandStore;

		public:
			MatchClientSession(Match& match, std::size_t sessionId, PlayerCommandStore& commandStore, std::unique_ptr<SessionBridge> bridge);
			MatchClientSession(const MatchClientSession&) = delete;
			MatchClientSession(MatchClientSession&&) = delete;
			~MatchClientSession();

			void Disconnect();

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
			void HandleIncomingPacket(const Packets::HelloWorld& packet);
			void HandleIncomingPacket(const Packets::PlayersInput& packet);
			void HandleIncomingPacket(const Packets::Ready& packet);

			Match& m_match;
			MatchClientVisibility m_visibility;
			PlayerCommandStore& m_commandStore;
			std::size_t m_sessionId;
			std::unique_ptr<SessionBridge> m_bridge;
			std::vector<Player> m_players;
	};
}

#include <Shared/MatchClientSession.inl>

#endif