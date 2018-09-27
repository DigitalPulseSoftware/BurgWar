// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/ServerConnection.hpp>
#include <Client/BurgApp.hpp>

namespace bw
{
	bool ServerConnection::Connect(const Nz::String& serverHostname, Nz::UInt32 data)
	{
		if (IsConnected())
			Disconnect(0);

		m_connected = false;
		return m_application.ConnectNewServer(serverHostname, data, this, &m_peerId, &m_networkReactor);
	}

	Nz::UInt64 ServerConnection::EstimateServerTime() const
	{
		return m_application.GetAppTime() + m_deltaTime;
	}

	void ServerConnection::UpdateNetworkStrings(ServerConnection* server, const Packets::NetworkStrings& data)
	{
		assert(server == this);

		m_stringStore.FillStore(data.startId, std::move(data.strings));
	}
}
