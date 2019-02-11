// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/LocalSessionBridge.hpp>
#include <Client/LocalSessionManager.hpp>
#include <iostream>

namespace bw
{
	void LocalSessionBridge::Disconnect()
	{
		m_sessionManager.DisconnectPeer(m_peerId);
	}

	void LocalSessionBridge::SendPacket(Nz::UInt8 /*channelId*/, Nz::ENetPacketFlags /*flags*/, Nz::NetPacket&& packet)
	{
		m_sessionManager.SendPacket(m_peerId, std::move(packet), m_isServer);
	}
}
