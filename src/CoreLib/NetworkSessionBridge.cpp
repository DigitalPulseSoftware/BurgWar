// Copyright (C) 2020 Jérôme Leclercq
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

	void NetworkSessionBridge::QueryInfo(std::function<void(const SessionInfo& info)> callback) const
	{
		m_reactor.QueryInfo(m_peerId, [callback = std::move(callback)](NetworkReactor::PeerInfo& peerInfo)
		{
			SessionInfo info;
			info.ping = peerInfo.ping;
			info.timeSinceLastReceive = peerInfo.timeSinceLastReceive;
			info.totalByteReceived = peerInfo.totalByteReceived;
			info.totalByteSent = peerInfo.totalByteSent;
			info.totalPacketReceived = peerInfo.totalPacketReceived;
			info.totalPacketLost = peerInfo.totalPacketLost;
			info.totalPacketSent = peerInfo.totalPacketSent;

			callback(info);
		});
	}

	void NetworkSessionBridge::SendPacket(Nz::UInt8 channelId, Nz::ENetPacketFlags flags, Nz::NetPacket && packet)
	{
		packet.FlushBits();
		m_reactor.SendData(m_peerId, channelId, flags, std::move(packet));
	}
}
