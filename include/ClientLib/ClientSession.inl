// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/ClientSession.hpp>
#include <CoreLib/BurgAppComponent.hpp>
#include <Nazara/Network/NetPacket.hpp>

namespace bw
{
	inline ClientSession::ClientSession(BurgAppComponent& application) :
	m_application(application),
	m_commandStore(m_application.GetLogger())
	{
	}

	inline BurgAppComponent& ClientSession::GetApp()
	{
		return m_application;
	}

	inline const BurgAppComponent& ClientSession::GetApp() const
	{
		return m_application;
	}

	inline const NetworkStringStore& ClientSession::GetNetworkStringStore() const
	{
		return m_stringStore;
	}

	inline bool ClientSession::IsConnected() const
	{
		return m_bridge && m_bridge->IsConnected();
	}

	inline void ClientSession::QuerySessionInfo(std::function<void(const SessionBridge::SessionInfo & info)> callback) const
	{
		if (!IsConnected())
			return;

		m_bridge->QueryInfo(std::move(callback));
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
}
