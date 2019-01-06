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

	std::size_t LocalSessionManager::CreateSession()
	{
		// Find first free peerId
		std::size_t peerId;
		for (; peerId < m_peerIdToSession.size(); ++peerId)
		{
			if (m_peerIdToSession[peerId] == nullptr)
				break;
		}

		if (peerId == m_peerIdToSession.size())
			m_peerIdToSession.emplace_back(nullptr);

		std::cout << "Local session #" << peerId << " created" << std::endl;
		m_peerIdToSession[peerId] = GetOwner()->CreateSession(std::make_unique<LocalSessionBridge>(*this, peerId));

		return peerId;
	}

	void LocalSessionManager::Poll()
	{
		// Nothing to do?
	}
}
