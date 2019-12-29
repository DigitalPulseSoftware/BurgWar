// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/NetworkSessionManager.hpp>
#include <CoreLib/MatchClientSession.hpp>
#include <CoreLib/NetworkSessionBridge.hpp>
#include <CoreLib/Match.hpp>
#include <CoreLib/MatchSessions.hpp>
#include <CoreLib/LogSystem/Logger.hpp>

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
		               [&](std::size_t peerId, Nz::NetPacket&& packet) { HandlePeerPacket(peerId, std::move(packet)); });
	}

	void NetworkSessionManager::HandlePeerConnection(bool /*outgoing*/, std::size_t peerId, Nz::UInt32 /*data*/)
	{
		bwLog(GetOwner()->GetMatch().GetLogger(), LogLevel::Info, "Peer #{0} connected", peerId);

		std::shared_ptr<NetworkSessionBridge> clientBridge = std::make_shared<NetworkSessionBridge>(m_reactor, peerId);

		MatchClientSession* session = GetOwner()->CreateSession(std::move(clientBridge));

		if (peerId >= m_peerIdToSession.size())
			m_peerIdToSession.resize(peerId + 1);

		m_peerIdToSession[peerId] = session;
	}

	void NetworkSessionManager::HandlePeerDisconnection(std::size_t peerId, Nz::UInt32 /*data*/)
	{
		bwLog(GetOwner()->GetMatch().GetLogger(), LogLevel::Info, "Peer #{0} disconnected", peerId);

		MatchClientSession*& session = m_peerIdToSession[peerId];
		assert(session);

		GetOwner()->DeleteSession(session);
		session = nullptr;
	}

	void NetworkSessionManager::HandlePeerPacket(std::size_t peerId, Nz::NetPacket&& packet)
	{
		MatchClientSession* session = m_peerIdToSession[peerId];
		assert(session);

		//bwLog(m_logger, LogLevel::Info, "Peer #{0} sent packet", peerId);
		session->HandleIncomingPacket(packet);
	}
}
