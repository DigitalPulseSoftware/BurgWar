// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Components/NetworkSyncComponent.hpp>

namespace bw
{
	inline NetworkSyncComponent::NetworkSyncComponent(std::string entityClass, const Ndk::EntityHandle& parent) :
	m_entityClass(entityClass),
	m_parent(parent)
	{
	}

	inline const std::string& NetworkSyncComponent::GetEntityClass() const
	{
		return m_entityClass;
	}

	inline const Ndk::EntityHandle & NetworkSyncComponent::GetParent() const
	{
		return m_parent;
	}

	inline void NetworkSyncComponent::Invalidate()
	{
		OnInvalidated(this);
	}
}
