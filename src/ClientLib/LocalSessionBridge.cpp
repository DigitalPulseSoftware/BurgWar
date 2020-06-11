// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/LocalSessionBridge.hpp>
#include <CoreLib/BurgApp.hpp>
#include <CoreLib/Match.hpp>
#include <ClientLib/LocalSessionManager.hpp>

namespace bw
{
	LocalSessionBridge::LocalSessionBridge(LocalSessionManager& sessionManager, std::size_t peerId, bool isServer) :
	SessionBridge(nullptr),
	m_peerId(peerId),
	m_sessionManager(sessionManager),
	m_isServer(isServer)
	{
		BurgApp& app = m_sessionManager.GetOwner()->GetMatch().GetApp();
		m_lastReceiveTime = app.GetAppTime();

		m_sessionInfo.ping = 0;
		m_sessionInfo.totalByteReceived = 0;
		m_sessionInfo.totalByteSent = 0;
		m_sessionInfo.totalPacketLost = 0;
		m_sessionInfo.totalPacketReceived = 0;
		m_sessionInfo.totalPacketSent = 0;

		HandleConnection(0);
	}

	void LocalSessionBridge::Disconnect()
	{
		assert(IsConnected());

		m_sessionManager.DisconnectPeer(m_peerId);
	}

	void LocalSessionBridge::HandleIncomingPacket(Nz::NetPacket& packet)
	{
		BurgApp& app = m_sessionManager.GetOwner()->GetMatch().GetApp();
		m_lastReceiveTime = app.GetAppTime();

		m_sessionInfo.totalByteReceived += packet.GetDataSize();
		m_sessionInfo.totalPacketReceived++;

		SessionBridge::HandleIncomingPacket(packet);
	}

	void LocalSessionBridge::QueryInfo(std::function<void(const SessionInfo& info)> callback) const
	{
		BurgApp& app = m_sessionManager.GetOwner()->GetMatch().GetApp();

		m_sessionInfo.timeSinceLastReceive = static_cast<Nz::UInt32>(app.GetAppTime() - m_lastReceiveTime);

		callback(m_sessionInfo);
	}

	void LocalSessionBridge::SendPacket(Nz::UInt8 /*channelId*/, Nz::ENetPacketFlags /*flags*/, Nz::NetPacket&& packet)
	{
		assert(IsConnected());

		m_sessionInfo.totalByteSent += packet.GetDataSize();
		m_sessionInfo.totalPacketSent++;

		m_sessionManager.SendPacket(m_peerId, std::move(packet), m_isServer);
	}
}
