// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/ClientSession.hpp>

namespace bw
{
	inline ClientSession::ClientSession(NetworkReactor& reactor, std::size_t peerId, std::size_t sessionId, ClientCommandStore& commandStore) :
	m_commandStore(commandStore),
	m_reactor(reactor),
	m_peerId(peerId),
	m_sessionId(sessionId)
	{
	}

	inline std::size_t ClientSession::GetPeerId() const
	{
		return m_peerId;
	}

	inline std::size_t ClientSession::GetSessionId() const
	{
		return m_sessionId;
	}

	template<typename T>
	void ClientSession::SendPacket(const T& packet)
	{
		const auto& command = m_commandStore.GetOutgoingCommand<T>();

		Nz::NetPacket data;
		m_commandStore.SerializePacket(data, packet);

		m_reactor.SendData(m_peerId, command.channelId, command.flags, std::move(data));
	}
}