// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/ClientSession.hpp>
#include <CoreLib/BurgAppComponent.hpp>
#include <CoreLib/NetworkSessionBridge.hpp>
#include <ClientLib/ClientMatch.hpp>
#include <ClientLib/LocalSessionBridge.hpp>
#include <ClientLib/LocalSessionManager.hpp>
#include <Nazara/Core/VirtualDirectory.hpp>
#include <Nazara/Network/Algorithm.hpp>

namespace bw
{
	ClientSession::~ClientSession() = default;

	bool ClientSession::Connect(std::shared_ptr<SessionBridge> sessionBridge)
	{
		assert(sessionBridge);

		Disconnect();

		m_bridge = std::move(sessionBridge);

		m_onDisconnectedSlot.Connect(m_bridge->OnDisconnected, [this](Nz::UInt32 /*data*/)
		{
			OnSessionDisconnected();
		});

		m_onIncomingPacketSlot.Connect(m_bridge->OnIncomingPacket, [this](Nz::NetPacket& packet)
		{
			HandleIncomingPacket(packet);
		});

		OnNetworkStrings.Connect([this](ClientSession*, const Packets::NetworkStrings& packet)
		{
			if (packet.startId == 0)
				m_stringStore.Clear(); //< Reset string store

			bwLog(GetApp().GetLogger(), LogLevel::Debug, "Received {} network strings", packet.strings.size());

			m_stringStore.FillStore(packet.startId, packet.strings);
		});

		if (!m_bridge->IsConnected())
		{
			m_onConnectedSlot.Connect(m_bridge->OnConnected, [this](Nz::UInt32 /*data*/)
			{
				OnSessionConnected();
			});
		}
		else
			OnSessionConnected();

		return true;
	}

	void ClientSession::Disconnect()
	{
		if (m_bridge)
		{
			m_bridge->Disconnect();
			OnSessionDisconnected();
		}
	}

	void ClientSession::HandleIncomingPacket(Nz::NetPacket& packet)
	{
		m_commandStore.UnserializePacket(this, packet);
	}

	void ClientSession::OnSessionConnected()
	{
		bwLog(m_application.GetLogger(), LogLevel::Info, "Connected");

		OnConnected(this);
	}

	void ClientSession::OnSessionDisconnected()
	{
		m_bridge.reset();
		OnDisconnected(this);
	}
}
