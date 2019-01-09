// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/LocalSessionBridge.hpp>

namespace bw
{
	inline LocalSessionBridge::LocalSessionBridge(LocalSessionManager& sessionManager, std::size_t peerId, bool isServer) :
	SessionBridge(nullptr),
	m_sessionManager(sessionManager),
	m_peerId(peerId),
	m_isServer(isServer)
	{
	}

	inline bool bw::LocalSessionBridge::IsServer() const
	{
		return m_isServer;
	}
}