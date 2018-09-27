// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/SessionManager.hpp>
#include <Server/ClientSession.hpp>
#include <iostream>

namespace bw
{
	SessionManager::SessionManager(Nz::UInt16 port, std::size_t maxClient) :
	m_reactor(0, Nz::NetProtocol_Any, port, maxClient),
	m_sessionPool(sizeof(ClientSession)),
	m_nextSessionId(0)
	{
	}

	SessionManager::~SessionManager()
	{
		for (ClientSession* session : m_peerIdToSession)
			m_sessionPool.Delete(session);
	}

	void SessionManager::Update()
	{
		m_reactor.Poll([&](bool outgoing, std::size_t peerId, Nz::UInt32 data) { HandlePeerConnection(outgoing, peerId, data); },
		               [&](std::size_t peerId, Nz::UInt32 data) { HandlePeerDisconnection(peerId, data); },
		               [&](std::size_t peerId, Nz::NetPacket&& packet) { HandlePeerPacket(peerId, std::move(packet)); },
		               [&](std::size_t peerId, const NetworkReactor::PeerInfo& peerInfo) { HandlePeerInfo(peerId, peerInfo); });
	}

	void SessionManager::HandlePeerConnection(bool outgoing, std::size_t peerId, Nz::UInt32 data)
	{
		std::size_t sessionId = m_nextSessionId++;
		ClientSession* session = m_sessionPool.New<ClientSession>(m_reactor, peerId, sessionId, m_commandStore);

		m_sessionIdToSession.insert_or_assign(sessionId, session);

		if (peerId >= m_peerIdToSession.size())
			m_peerIdToSession.resize(peerId + 1);

		m_peerIdToSession[peerId] = session;

		std::cout << "Peer #" << peerId << " connected" << std::endl;
	}

	void SessionManager::HandlePeerDisconnection(std::size_t peerId, Nz::UInt32 data)
	{
		ClientSession*& session = m_peerIdToSession[peerId];
		assert(session);

		m_sessionIdToSession.erase(session->GetSessionId());

		m_sessionPool.Delete(session);
		session = nullptr;

		std::cout << "Peer #" << peerId << " disconnected" << std::endl;
	}

	void SessionManager::HandlePeerInfo(std::size_t peerId, const NetworkReactor::PeerInfo& peerInfo)
	{
		ClientSession* session = m_peerIdToSession[peerId];
		assert(session);

		std::cout << "Peer #" << peerId << " refreshed info" << std::endl;
	}

	void SessionManager::HandlePeerPacket(std::size_t peerId, Nz::NetPacket&& packet)
	{
		ClientSession* session = m_peerIdToSession[peerId];
		assert(session);

		std::cout << "Peer #" << peerId << " sent packet" << std::endl;
		session->HandlePacket(std::move(packet));
	}
}
