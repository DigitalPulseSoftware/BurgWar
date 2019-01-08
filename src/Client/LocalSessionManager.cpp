// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/LocalSessionManager.hpp>
#include <Client/LocalSessionBridge.hpp>
#include <Shared/MatchSessions.hpp>
#include <iostream>

namespace bw
{
	LocalSessionManager::LocalSessionManager(MatchSessions* owner) :
	SessionManager(owner)
	{
	}

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

		std::cout << "Local session #" << peerId << " created" << std::endl;
		auto& peer = m_peers.emplace_back();
		peer.emplace();
		peer->clientBridge = std::make_shared<LocalSessionBridge>(*this, peerId);
		peer->session = GetOwner()->CreateSession(std::make_unique<LocalSessionBridge>(*this, peerId));

		return peer->clientBridge;
	}

	void LocalSessionManager::Poll()
	{
		// TODO
	}

	void LocalSessionManager::SendPacket(std::size_t peerId, Nz::NetPacket&& packet)
	{
		assert(peerId < m_peers.size() && m_peers[peerId]);
		Peer& peer = m_peers[peerId].value();
		peer.pendingPackets.emplace_back(std::move(packet));
	}
}
