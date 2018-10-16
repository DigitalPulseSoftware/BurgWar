// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/Systems/NetworkSyncSystem.hpp>

namespace bw
{
	inline void NetworkSyncSystem::ClearEvents()
	{
		m_events.clear();
	}

	inline const std::vector<NetworkSyncSystem::Event>& NetworkSyncSystem::GetEvents() const
	{
		return m_events;
	}
}
