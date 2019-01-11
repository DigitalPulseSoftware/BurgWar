// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/NetworkSessionManager.hpp>
#include <Shared/MatchClientSession.hpp>
#include <Shared/NetworkSessionBridge.hpp>
#include <Shared/MatchSessions.hpp>
#include <iostream>

namespace bw
{
	NetworkSessionManager::NetworkSessionManager(MatchSessions* owner, Nz::UInt16 port, std::size_t maxClient) :
	SessionManager(owner),
	m_reactor(0, Nz::NetProtocol_Any, port, maxClient)
	{
	}

	NetworkSessionManager::~NetworkSessionManager() = default;

	void NetworkSessionManager::Poll()
	{
		m_reactor.Poll([&](bool outgoing, std::size_t peerId, Nz::UInt32 data) { HandlePeerConnection(outgoing, peerId, data); },
		               [&](std::size_t peerId, Nz::UInt32 data) { HandlePeerDisconnection(peerId, data); },
		               [&](std::size_t peerId, Nz::NetPacket&& packet) { HandlePeerPacket(peerId, std::move(packet)); },
		               [&](std::size_t peerId, const NetworkReactor::PeerInfo& peerInfo) { HandlePeerInfo(peerId, peerInfo); });
	}

	void NetworkSessionManager::HandlePeerConnection(bool outgoing, std::size_t peerId, Nz::UInt32 data)
	{
		std::cout << "Peer #" << peerId << " connected" << std::endl;

		std::shared_ptr<NetworkSessionBridge> clientBridge = std::make_shared<NetworkSessionBridge>(m_reactor, peerId);

		MatchClientSession* session = GetOwner()->CreateSession(std::move(clientBridge));

		if (peerId >= m_peerIdToSession.size())
			m_peerIdToSession.resize(peerId + 1);

		m_peerIdToSession[peerId] = session;
	}

	void NetworkSessionManager::HandlePeerDisconnection(std::size_t peerId, Nz::UInt32 data)
	{
		std::cout << "Peer #" << peerId << " disconnected" << std::endl;

		MatchClientSession*& session = m_peerIdToSession[peerId];
		assert(session);

		GetOwner()->DeleteSession(session);
		session = nullptr;
	}

	void NetworkSessionManager::HandlePeerInfo(std::size_t peerId, const NetworkReactor::PeerInfo& peerInfo)
	{
		MatchClientSession* session = m_peerIdToSession[peerId];
		assert(session);

		std::cout << "Peer #" << peerId << " refreshed info" << std::endl;
	}

	void NetworkSessionManager::HandlePeerPacket(std::size_t peerId, Nz::NetPacket&& packet)
	{
		MatchClientSession* session = m_peerIdToSession[peerId];
		assert(session);

		std::cout << "Peer #" << peerId << " sent packet" << std::endl;
		session->HandleIncomingPacket(packet);
	}
}
