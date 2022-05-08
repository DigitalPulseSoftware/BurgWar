// Copyright (C) 2016 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in Prerequisites.hpp

#include <CoreLib/EntityOwner.hpp>

namespace bw
{
	inline EntityOwner::EntityOwner(entt::registry& registry, entt::entity entity) :
	m_entity(entity),
	m_registry(registry)
	{
	}
	
	inline EntityOwner::EntityOwner(EntityOwner&& entityOwner) noexcept :
	m_entity(entityOwner.m_entity),
	m_registry(entityOwner.m_registry)
	{
	}

	inline EntityOwner::~EntityOwner()
	{
		m_registry.destroy(m_entity);
	}

	inline entt::entity EntityOwner::GetEntity() const
	{
		return m_entity;
	}
	
	inline EntityOwner::operator entt::entity() const
	{
		return m_entity;
	}
}
