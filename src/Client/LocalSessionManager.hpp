// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENT_LOCALSESSIONMANAGER_HPP
#define BURGWAR_CLIENT_LOCALSESSIONMANAGER_HPP

#include <Shared/SessionManager.hpp>
#include <Nazara/Core/MemoryPool.hpp>
#include <optional>
#include <vector>

namespace bw
{
	class LocalSessionBridge;
	class MatchClientSession;
	class MatchSessions;

	class LocalSessionManager : public SessionManager
	{
		friend LocalSessionBridge;

		public:
			LocalSessionManager(MatchSessions* owner);
			~LocalSessionManager();

			std::shared_ptr<LocalSessionBridge> CreateSession();

			void Poll() override;

		private:
			void SendPacket(std::size_t peerId, Nz::NetPacket&& packet);

			struct Peer
			{
				std::shared_ptr<LocalSessionBridge> clientBridge;
				std::vector<Nz::NetPacket> pendingPackets;
				MatchClientSession* session;
			};

			std::vector<std::optional<Peer>> m_peers;
	};
}

#include <Shared/NetworkSessionManager.inl>

#endif