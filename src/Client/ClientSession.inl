// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/ClientSession.hpp>
#include <Nazara/Network/NetPacket.hpp>

namespace bw
{
	inline ClientSession::ClientSession(ClientApp& application, const LocalCommandStore& commandStore) :
	m_application(application),
	m_commandStore(commandStore),
	m_isConnected(false)
	{
	}

	inline ClientApp& ClientSession::GetApp()
	{
		return m_application;
	}

	inline const ClientApp& ClientSession::GetApp() const
	{
		return m_application;
	}

	inline const ClientSession::ConnectionInfo& ClientSession::GetConnectionInfo() const
	{
		return m_connectionInfo;
	}

	inline const NetworkStringStore& ClientSession::GetNetworkStringStore() const
	{
		return m_stringStore;
	}

	inline bool ClientSession::IsConnected() const
	{
		return m_isConnected;
	}

	template<typename T>
	void ClientSession::SendPacket(const T& packet)
	{
		if (!IsConnected())
			return;

		Nz::NetPacket data;
		m_commandStore.SerializePacket(data, packet);

		const auto& command = m_commandStore.GetOutgoingCommand<T>();
		m_bridge->SendPacket(command.channelId, command.flags, std::move(data));
	}

	inline void ClientSession::UpdateInfo(const ConnectionInfo& connectionInfo)
	{
		OnConnectionInfoUpdate(this, connectionInfo);
		m_connectionInfo = connectionInfo;
	}
}
