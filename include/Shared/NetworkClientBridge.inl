// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/NetworkClientBridge.hpp>

namespace bw
{
	inline NetworkClientBridge::NetworkClientBridge(NetworkReactor& reactor, std::size_t peerId) :
	SessionBridge(nullptr),
	m_peerId(peerId),
	m_reactor(reactor)
	{
	}

	inline std::size_t NetworkClientBridge::GetPeerId() const
	{
		return m_peerId;
	}
}