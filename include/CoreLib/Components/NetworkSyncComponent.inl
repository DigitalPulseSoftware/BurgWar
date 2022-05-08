// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Components/NetworkSyncComponent.hpp>

namespace bw
{
	inline NetworkSyncComponent::NetworkSyncComponent(std::string entityClass, entt::entity parent) :
	m_parent(parent),
	m_entityClass(entityClass)
	{
	}

	inline const std::string& NetworkSyncComponent::GetEntityClass() const
	{
		return m_entityClass;
	}

	inline entt::entity NetworkSyncComponent::GetParent() const
	{
		return m_parent;
	}

	inline void NetworkSyncComponent::Invalidate()
	{
		OnInvalidated(this);
	}
	
	inline void NetworkSyncComponent::UpdateParent(entt::entity parent)
	{
		m_parent = parent;
		//TODO: network event
	}
}
