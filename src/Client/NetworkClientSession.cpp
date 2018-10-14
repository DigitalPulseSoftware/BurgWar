// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/NetworkClientSession.hpp>
#include <Client/BurgApp.hpp>

namespace bw
{
	bool NetworkClientSession::Connect(const Nz::String& serverHostname, Nz::UInt32 data)
	{
		if (IsConnected())
			Disconnect(0);

		m_connected = false;
		return m_application.ConnectNewServer(serverHostname, data, this, &m_peerId, &m_networkReactor);
	}

	Nz::UInt64 NetworkClientSession::EstimateServerTime() const
	{
		return m_application.GetAppTime() + m_deltaTime;
	}

	void NetworkClientSession::UpdateNetworkStrings(NetworkClientSession* server, const Packets::NetworkStrings& data)
	{
		assert(server == this);

		m_stringStore.FillStore(data.startId, std::move(data.strings));
	}
}
