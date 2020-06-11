// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Components/WeaponComponent.hpp>

namespace bw
{
	inline WeaponComponent::WeaponComponent(Ndk::EntityHandle owner, WeaponAttackMode attackMode) :
	m_owner(std::move(owner)),
	m_attackMode(attackMode),
	m_isActive(false),
	m_isAttacking(false)
	{
	}

	inline WeaponAttackMode WeaponComponent::GetAttackMode() const
	{
		return m_attackMode;
	}

	inline const Ndk::EntityHandle& WeaponComponent::GetOwner() const
	{
		return m_owner;
	}

	inline bool WeaponComponent::IsActive() const
	{
		return m_isActive;
	}

	inline bool WeaponComponent::IsAttacking() const
	{
		return m_isAttacking;
	}

	inline void WeaponComponent::SetActive(bool isActive)
	{
		m_isActive = isActive;
		if (!isActive)
			m_isAttacking = false;
	}

	inline void WeaponComponent::SetAttacking(bool isAttacking)
	{
		m_isAttacking = isAttacking;
	}

	inline void WeaponComponent::UpdateOwner(Ndk::EntityHandle owner)
	{
		m_owner = std::move(owner);
	}
}
