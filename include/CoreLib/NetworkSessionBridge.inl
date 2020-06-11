// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/NetworkSessionBridge.hpp>

namespace bw
{
	inline NetworkSessionBridge::NetworkSessionBridge(NetworkReactor& reactor, std::size_t peerId) :
	SessionBridge(nullptr),
	m_peerId(peerId),
	m_reactor(reactor)
	{
	}

	inline std::size_t NetworkSessionBridge::GetPeerId() const
	{
		return m_peerId;
	}
}
