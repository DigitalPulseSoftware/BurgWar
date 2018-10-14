// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/LocalSessionBridge.hpp>
#include <iostream>

namespace bw
{
	void LocalSessionBridge::Disconnect()
	{
		throw std::runtime_error("Disconnect() called on local player");
	}

	void LocalSessionBridge::SendPacket(const PlayerCommandStore::OutgoingCommand& command, Nz::NetPacket && packet)
	{
		//m_reactor.SendData(m_peerId, command.channelId, command.flags, std::move(packet));
	}
}
