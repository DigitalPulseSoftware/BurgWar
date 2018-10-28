// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/MatchClientSession.hpp>

namespace bw
{
	inline MatchClientSession::MatchClientSession(Match& match, std::size_t sessionId, PlayerCommandStore& commandStore,  std::unique_ptr<SessionBridge> bridge) :
	m_match(match),
	m_visibility(match, *this),
	m_commandStore(commandStore),
	m_sessionId(sessionId),
	m_bridge(std::move(bridge))
	{
	}

	inline std::size_t MatchClientSession::GetSessionId() const
	{
		return m_sessionId;
	}

	inline MatchClientVisibility& MatchClientSession::GetVisibility()
	{
		return m_visibility;
	}

	inline const MatchClientVisibility& MatchClientSession::GetVisibility() const
	{
		return m_visibility;
	}

	template<typename T>
	void MatchClientSession::SendPacket(const T& packet)
	{
		Nz::NetPacket data;
		m_commandStore.SerializePacket(data, packet);

		const auto& command = m_commandStore.GetOutgoingCommand<T>();
		m_bridge->SendPacket(command.channelId, command.flags, std::move(data));
	}
}