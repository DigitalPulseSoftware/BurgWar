// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_SERVER_CLIENTSESSION_HPP
#define BURGWAR_SERVER_CLIENTSESSION_HPP

#include <GameLibShared/PlayerCommandStore.hpp>
#include <GameLibShared/SessionBridge.hpp>
#include <GameLibShared/Protocol/Packets.hpp>
#include <memory>
#include <vector>

namespace bw
{
	class Match;
	class MatchClientVisibility;
	class Player;
	class PlayerCommandStore;

	// Match client session
	class MatchClientSession final
	{
		friend PlayerCommandStore;

		public:
			MatchClientSession(Match& match, std::size_t sessionId, PlayerCommandStore& commandStore, std::shared_ptr<SessionBridge> bridge);
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
			PlayerCommandStore& m_commandStore;
			std::size_t m_sessionId;
			std::shared_ptr<SessionBridge> m_bridge;
			std::unique_ptr<MatchClientVisibility> m_visibility;
			std::vector<Player> m_players;
	};
}

#include <GameLibShared/MatchClientSession.inl>

#endif