// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_LOCALSESSIONMANAGER_HPP
#define BURGWAR_CLIENTLIB_LOCALSESSIONMANAGER_HPP

#include <CoreLib/SessionManager.hpp>
#include <ClientLib/Export.hpp>
#include <Nazara/Core/MemoryPool.hpp>
#include <optional>
#include <vector>

namespace bw
{
	class LocalSessionBridge;
	class MatchClientSession;
	class MatchSessions;

	class BURGWAR_CLIENTLIB_API LocalSessionManager : public SessionManager
	{
		friend LocalSessionBridge;

		public:
			using SessionManager::SessionManager;
			LocalSessionManager(const LocalSessionManager&) = delete;
			LocalSessionManager(LocalSessionManager&&) = delete;
			~LocalSessionManager();

			std::shared_ptr<LocalSessionBridge> CreateSession();

			void Poll() override;

			LocalSessionManager& operator=(const LocalSessionManager&) = delete;
			LocalSessionManager& operator=(LocalSessionManager&&) = delete;

		private:
			void DisconnectPeer(std::size_t peerId);
			void SendPacket(std::size_t peerId, Nz::NetPacket&& packet, bool isServer);

			struct Peer
			{
				std::shared_ptr<LocalSessionBridge> clientBridge;
				std::shared_ptr<LocalSessionBridge> serverBridge;
				std::vector<Nz::NetPacket> clientPackets;
				std::vector<Nz::NetPacket> serverPackets;
				MatchClientSession* session;
				bool disconnectionRequested = false;
			};

			std::vector<std::optional<Peer>> m_peers;
	};
}

#include <CoreLib/NetworkSessionManager.inl>

#endif
