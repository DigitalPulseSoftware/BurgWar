// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Components/NetworkSyncComponent.hpp>
#include <utility>

namespace bw
{
	inline NetworkSyncComponent::NetworkSyncComponent(NetworkSyncComponent&& networkComponent) noexcept :
	m_entityClass(std::move(networkComponent.m_entityClass)),
	m_parent(std::move(networkComponent.m_parent)),
	m_networkId(std::exchange(networkComponent.m_networkId, InvalidNetworkId)),
	m_networkSystem(std::move(networkComponent.m_networkSystem))
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
	
	inline NetworkSyncComponent& NetworkSyncComponent::operator=(NetworkSyncComponent&& networkComponent) noexcept
	{
		m_entityClass = std::move(networkComponent.m_entityClass);
		m_parent = std::move(networkComponent.m_parent);
		m_networkSystem = std::move(networkComponent.m_networkSystem);

		std::swap(m_networkId, networkComponent.m_networkId);

		return *this;
	}
}
