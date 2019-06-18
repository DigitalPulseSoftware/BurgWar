// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Components/WeaponComponent.hpp>

namespace bw
{
	inline WeaponComponent::WeaponComponent(Ndk::EntityHandle owner) :
	m_owner(std::move(owner)),
	m_isActive(false)
	{
	}

	inline const Ndk::EntityHandle& WeaponComponent::GetOwner() const
	{
		return m_owner;
	}

	inline bool WeaponComponent::IsActive() const
	{
		return m_isActive;
	}

	inline void WeaponComponent::SetActive(bool isActive)
	{
		m_isActive = isActive;
	}
}
