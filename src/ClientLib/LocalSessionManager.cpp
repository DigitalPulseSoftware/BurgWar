// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/LocalSessionManager.hpp>
#include <ClientLib/LocalSessionBridge.hpp>
#include <CoreLib/Match.hpp>
#include <CoreLib/MatchSessions.hpp>
#include <CoreLib/LogSystem/Logger.hpp>

namespace bw
{
	LocalSessionManager::~LocalSessionManager() = default;

	std::shared_ptr<LocalSessionBridge> LocalSessionManager::CreateSession()
	{
		// Find first free peerId
		std::size_t peerId = 0;
		for (; peerId < m_peers.size(); ++peerId)
		{
			if (!m_peers[peerId].has_value())
				break;
		}

		if (peerId == m_peers.size())
			m_peers.emplace_back();

		bwLog(GetOwner()->GetMatch().GetLogger(), LogLevel::Info, "Local session #{0} created", peerId);
		auto& peer = m_peers[peerId];
		peer.emplace();
		peer->clientBridge = std::make_shared<LocalSessionBridge>(*this, peerId, false);
		peer->serverBridge = std::make_shared<LocalSessionBridge>(*this, peerId, true);
		peer->session = GetOwner()->CreateSession(peer->serverBridge);

		return peer->clientBridge;
	}

	void LocalSessionManager::Poll()
	{
		for (auto& peerOpt : m_peers)
		{
			if (peerOpt)
			{
				Peer& peer = peerOpt.value();
				for (auto&& packet : peer.clientPackets)
					peer.clientBridge->HandleIncomingPacket(packet);

				peer.clientPackets.clear();

				for (auto&& packet : peer.serverPackets)
					peer.serverBridge->HandleIncomingPacket(packet);

				peer.serverPackets.clear();

				if (peer.disconnectionRequested)
				{
					peer.clientBridge->HandleDisconnection(0);
					peer.serverBridge->HandleDisconnection(0);

					GetOwner()->DeleteSession(peer.session);

					peerOpt.reset();
				}
			}
		}
	}

	void LocalSessionManager::DisconnectPeer(std::size_t peerId)
	{
		assert(peerId < m_peers.size() && m_peers[peerId]);
		Peer& peer = m_peers[peerId].value();
		peer.disconnectionRequested = true;
	}

	void LocalSessionManager::SendPacket(std::size_t peerId, Nz::ByteArray&& packet, bool isServer)
	{
		assert(peerId < m_peers.size() && m_peers[peerId]);
		Peer& peer = m_peers[peerId].value();

		// Reset cursor position
		if (isServer)
			peer.clientPackets.emplace_back(std::move(packet));
		else
			peer.serverPackets.emplace_back(std::move(packet));
	}
}
