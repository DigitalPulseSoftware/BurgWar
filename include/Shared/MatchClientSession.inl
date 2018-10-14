// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/MatchClientSession.hpp>

namespace bw
{
	inline MatchClientSession::MatchClientSession(std::size_t sessionId, PlayerCommandStore& commandStore,  std::unique_ptr<SessionBridge> bridge) :
	m_commandStore(commandStore),
	m_sessionId(sessionId),
	m_bridge(std::move(bridge))
	{
	}

	inline std::size_t MatchClientSession::GetSessionId() const
	{
		return m_sessionId;
	}

	template<typename T>
	void MatchClientSession::SendPacket(const T& packet)
	{
		Nz::NetPacket data;
		m_commandStore.SerializePacket(data, packet);

		m_bridge->SendPacket(m_commandStore.GetOutgoingCommand<T>(), std::move(data));
	}
}