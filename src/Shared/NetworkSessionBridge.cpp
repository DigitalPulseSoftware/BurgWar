// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/NetworkSessionBridge.hpp>
#include <Shared/NetworkReactor.hpp>
#include <iostream>

namespace bw
{
	NetworkSessionBridge::~NetworkSessionBridge() = default;

	void NetworkSessionBridge::Disconnect()
	{
		m_reactor.DisconnectPeer(m_peerId);
	}

	void NetworkSessionBridge::SendPacket(Nz::UInt8 channelId, Nz::ENetPacketFlags flags, Nz::NetPacket && packet)
	{
		packet.FlushBits();
		m_reactor.SendData(m_peerId, channelId, flags, std::move(packet));
	}
}
