// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Components/NetworkSyncComponent.hpp>
#include <CoreLib/Systems/NetworkSyncSystem.hpp>

namespace bw
{
	NetworkSyncComponent::NetworkSyncComponent(NetworkSyncSystem& networkSystem, std::string entityClass, entt::handle parent) :
	m_entityClass(entityClass),
	m_parent(parent),
	m_networkSystem(&networkSystem)
	{
		m_networkId = m_networkSystem->AllocateNetworkId();
	}

	NetworkSyncComponent::~NetworkSyncComponent()
	{
		if (m_networkId != InvalidNetworkId)
			m_networkSystem->FreeNetworkId(m_networkId);
	}
}
