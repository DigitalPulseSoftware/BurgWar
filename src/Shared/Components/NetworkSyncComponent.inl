// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/Components/NetworkSyncComponent.hpp>

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
}
