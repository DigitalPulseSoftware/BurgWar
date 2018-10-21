// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/NetworkClientBridge.hpp>
#include <Shared/NetworkReactor.hpp>
#include <iostream>

namespace bw
{
	NetworkClientBridge::~NetworkClientBridge() = default;

	void NetworkClientBridge::Disconnect()
	{
		m_reactor.DisconnectPeer(m_peerId);
	}

	void NetworkClientBridge::SendPacket(Nz::UInt8 channelId, Nz::ENetPacketFlags flags, Nz::NetPacket && packet)
	{
		packet.FlushBits();
		m_reactor.SendData(m_peerId, channelId, flags, std::move(packet));
	}
}
