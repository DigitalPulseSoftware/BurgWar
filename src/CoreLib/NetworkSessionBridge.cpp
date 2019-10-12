// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/NetworkSessionBridge.hpp>
#include <CoreLib/NetworkReactor.hpp>

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
