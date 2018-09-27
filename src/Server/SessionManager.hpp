// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_SERVER_SESSIONMANAGER_HPP
#define BURGWAR_SERVER_SESSIONMANAGER_HPP

#include <Shared/NetworkReactor.hpp>
#include <Server/ClientCommandStore.hpp>
#include <Nazara/Core/MemoryPool.hpp>
#include <hopstotch/hopscotch_map.h>

namespace bw
{
	class ClientSession;

	class SessionManager
	{
		public:
			SessionManager(Nz::UInt16 port, std::size_t maxClient);
			~SessionManager();

			void Update();

		private:
			void HandlePeerConnection(bool outgoing, std::size_t peerId, Nz::UInt32 data);
			void HandlePeerDisconnection(std::size_t peerId, Nz::UInt32 data);
			void HandlePeerInfo(std::size_t peerId, const NetworkReactor::PeerInfo& peerInfo);
			void HandlePeerPacket(std::size_t peerId, Nz::NetPacket&& packet);

			std::vector<ClientSession*> m_peerIdToSession;
			ClientCommandStore m_commandStore;
			NetworkReactor m_reactor;
			Nz::MemoryPool m_sessionPool;
			std::size_t m_nextSessionId;
			tsl::hopscotch_map<std::size_t /*sessionId*/, ClientSession* /*session*/> m_sessionIdToSession;
	};
}

#include <Server/SessionManager.inl>

#endif