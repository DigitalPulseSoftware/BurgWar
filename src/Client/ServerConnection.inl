// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/ServerConnection.hpp>
#include <Nazara/Network/NetPacket.hpp>
#include <Shared/NetworkReactor.hpp>

namespace bw
{
	inline ServerConnection::ServerConnection(BurgApp& application, const ServerCommandStore& commandStore) :
	m_application(application),
	m_commandStore(commandStore),
	m_networkReactor(nullptr),
	m_peerId(NetworkReactor::InvalidPeerId),
	m_connected(false)
	{
		OnNetworkStrings.Connect([this](ServerConnection* server, const Packets::NetworkStrings& data) { UpdateNetworkStrings(server, data); });
	}

	inline void ServerConnection::Disconnect(Nz::UInt32 data)
	{
		m_networkReactor->DisconnectPeer(m_peerId, data);
	}

	inline BurgApp& ServerConnection::GetApp()
	{
		return m_application;
	}

	inline const BurgApp& ServerConnection::GetApp() const
	{
		return m_application;
	}

	inline const ServerConnection::ConnectionInfo& ServerConnection::GetConnectionInfo() const
	{
		return m_connectionInfo;
	}

	inline const NetworkStringStore& ServerConnection::GetNetworkStringStore() const
	{
		return m_stringStore;
	}

	inline std::size_t ServerConnection::GetPeerId() const
	{
		return m_peerId;
	}

	inline bool ServerConnection::IsConnected() const
	{
		return m_connected;
	}

	inline void ServerConnection::RefreshInfos()
	{
		if (!IsConnected())
			return;

		m_networkReactor->QueryInfo(m_peerId);
	}

	inline void ServerConnection::UpdateServerTimeDelta(Nz::UInt64 deltaTime)
	{
		m_deltaTime = deltaTime;
	}

	template<typename T>
	void ServerConnection::SendPacket(const T& packet)
	{
		if (!IsConnected())
			return;

		const auto& command = m_commandStore.GetOutgoingCommand<T>();

		Nz::NetPacket data;
		m_commandStore.SerializePacket(data, packet);

		m_networkReactor->SendData(m_peerId, command.channelId, command.flags, std::move(data));
	}

	inline void ServerConnection::DispatchIncomingPacket(Nz::NetPacket&& packet)
	{
		m_commandStore.UnserializePacket(this, std::move(packet));
	}

	inline void ServerConnection::NotifyConnected(Nz::UInt32 data)
	{
		m_connected = true;

		OnConnected(this, data);
	}

	inline void ServerConnection::NotifyDisconnected(Nz::UInt32 data)
	{
		m_connected = false;
		m_peerId = NetworkReactor::InvalidPeerId;
		m_stringStore.Clear();

		OnDisconnected(this, data);
	}

	inline void ServerConnection::UpdateInfo(const ConnectionInfo& connectionInfo)
	{
		OnConnectionInfoUpdate(this, connectionInfo);
		m_connectionInfo = connectionInfo;
	}
}
