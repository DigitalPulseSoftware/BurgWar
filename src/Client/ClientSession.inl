// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/ClientSession.hpp>
#include <Nazara/Network/NetPacket.hpp>

namespace bw
{
	inline ClientSession::ClientSession(BurgApp& application, const ServerCommandStore& commandStore) :
	m_application(application),
	m_commandStore(commandStore)
	{
	}

	inline BurgApp& ClientSession::GetApp()
	{
		return m_application;
	}

	inline const BurgApp& ClientSession::GetApp() const
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
		return m_bridge != nullptr;
	}

	template<typename T>
	void ClientSession::SendPacket(const T& packet)
	{
		if (!m_isConnected)
			return;

		Nz::NetPacket data;
		m_commandStore.SerializePacket(data, packet);

		m_bridge->SendPacket(m_commandStore.GetOutgoingCommand<T>(), std::move(data));
	}

	inline void ClientSession::UpdateInfo(const ConnectionInfo& connectionInfo)
	{
		OnConnectionInfoUpdate(this, connectionInfo);
		m_connectionInfo = connectionInfo;
	}
}
