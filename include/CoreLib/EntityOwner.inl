// Copyright (C) 2016 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in Prerequisites.hpp

#include <CoreLib/EntityOwner.hpp>

namespace bw
{
	inline EntityOwner::EntityOwner(entt::handle entity) :
	m_entity(entity)
	{
	}
	
	inline EntityOwner::EntityOwner(EntityOwner&& entityOwner) noexcept :
	m_entity(entityOwner.m_entity)
	{
	}

	inline EntityOwner::~EntityOwner()
	{
		if (m_entity.valid())
			m_entity.destroy();
	}

	inline entt::handle EntityOwner::GetEntity() const
	{
		return m_entity;
	}
	
	inline EntityOwner::operator entt::handle() const
	{
		return m_entity;
	}

	inline entt::handle* EntityOwner::operator->()
	{
		return &m_entity;
	}

	inline const entt::handle* EntityOwner::operator->() const
	{
		return &m_entity;
	}

	inline EntityOwner& EntityOwner::operator=(EntityOwner&& entityOwner) noexcept
	{
		m_entity = entityOwner.m_entity;

		return *this;
	}
}
