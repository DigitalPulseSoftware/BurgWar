// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/LocalSessionBridge.hpp>

namespace bw
{
	inline bool LocalSessionBridge::IsServer() const
	{
		return m_isServer;
	}
}
