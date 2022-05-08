// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Components/BaseComponent.hpp>
#include <cassert>

namespace bw
{
	inline BaseComponent::BaseComponent(entt::registry& registry, entt::entity entity) :
	m_entity(entity),
	m_registry(registry)
	{
	}

	inline BaseComponent::BaseComponent(const BaseComponent& component) :
	m_entity(component.m_entity),
	m_registry(component.m_registry)
	{
	}

	inline BaseComponent::BaseComponent(BaseComponent&& component) noexcept :
	m_entity(component.m_entity),
	m_registry(component.m_registry)
	{
	}

	inline entt::entity BaseComponent::GetEntity() const
	{
		return m_entity;
	}

	inline entt::registry& BaseComponent::GetRegistry()
	{
		return m_registry;
	}

	inline entt::registry& BaseComponent::GetRegistry() const
	{
		return m_registry;
	}

	inline void BaseComponent::KillEntity()
	{
		m_registry.destroy(m_entity);
	}
}
