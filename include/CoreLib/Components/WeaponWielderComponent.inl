// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Components/WeaponWielderComponent.hpp>

namespace bw
{
	inline WeaponWielderComponent::WeaponWielderComponent() :
	m_weaponOffset(Nz::Vector2f::Zero())
	{
	}
	
	inline const Nz::Vector2f& WeaponWielderComponent::GetWeaponOffset() const
	{
		return m_weaponOffset;
	}
	
	inline void WeaponWielderComponent::SetWeaponOffset(const Nz::Vector2f& weaponOffset)
	{
		m_weaponOffset = weaponOffset;
	}
}
