// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/NetworkClientBridge.hpp>
#include <Shared/NetworkReactor.hpp>
#include <iostream>

namespace bw
{
	NetworkClientBridge::~NetworkClientBridge()
	{
		Disconnect();
	}

	void NetworkClientBridge::Disconnect()
	{
		m_reactor.DisconnectPeer(m_peerId);
	}

	void NetworkClientBridge::SendPacket(const PlayerCommandStore::OutgoingCommand& command, Nz::NetPacket && packet)
	{
		m_reactor.SendData(m_peerId, command.channelId, command.flags, std::move(packet));
	}
}
