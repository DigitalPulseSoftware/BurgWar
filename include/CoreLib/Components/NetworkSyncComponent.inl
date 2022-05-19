// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Components/NetworkSyncComponent.hpp>

namespace bw
{
	inline NetworkSyncComponent::NetworkSyncComponent(Nz::UInt32 networkId, std::string entityClass, entt::handle parent) :
	m_entityClass(entityClass),
	m_parent(parent),
	m_networkId(networkId)
	{
	}

	inline const std::string& NetworkSyncComponent::GetEntityClass() const
	{
		return m_entityClass;
	}

	inline Nz::UInt32 NetworkSyncComponent::GetNetworkId() const
	{
		return m_networkId;
	}

	inline entt::handle NetworkSyncComponent::GetParent() const
	{
		return m_parent;
	}

	inline void NetworkSyncComponent::Invalidate()
	{
		OnInvalidated(this);
	}
	
	inline void NetworkSyncComponent::UpdateParent(entt::handle parent)
	{
		m_parent = parent;
		//TODO: network event
	}
}
