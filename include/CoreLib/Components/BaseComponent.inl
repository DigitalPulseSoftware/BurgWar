// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Components/BaseComponent.hpp>
#include <cassert>

namespace bw
{
	inline BaseComponent::BaseComponent(entt::handle handle) :
	m_handle(handle)
	{
	}

	inline entt::entity BaseComponent::GetEntity() const
	{
		return m_handle;
	}

	inline entt::handle BaseComponent::GetHandle() const
	{
		return m_handle;
	}

	inline entt::registry* BaseComponent::GetRegistry()
	{
		return m_handle.registry();
	}

	inline entt::registry* BaseComponent::GetRegistry() const
	{
		return m_handle.registry();
	}

	inline void BaseComponent::KillEntity()
	{
		m_handle.destroy();
	}
}
