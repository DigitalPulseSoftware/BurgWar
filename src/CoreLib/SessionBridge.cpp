// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/SessionBridge.hpp>

namespace bw
{
	SessionBridge::~SessionBridge() = default;

	void SessionBridge::HandleConnection(Nz::UInt32 data)
	{
		m_isConnected = true;

		OnConnected(data);
	}

	void SessionBridge::HandleDisconnection(Nz::UInt32 data)
	{
		m_isConnected = false;

		OnDisconnected(data);
	}

	void SessionBridge::HandleIncomingPacket(Nz::NetPacket& packet)
	{
		assert(m_isConnected);

		OnIncomingPacket(packet);
	}
}
