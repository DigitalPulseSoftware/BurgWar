// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Components/WeaponWielderComponent.hpp>

namespace bw
{
	inline WeaponWielderComponent::WeaponWielderComponent() :
	m_weaponOffset(Nz::Vector2f::Zero()),
	m_activeWeaponIndex(NoWeapon)
	{
	}

	inline WeaponWielderComponent::WeaponWielderComponent(const WeaponWielderComponent& weaponWielder) :
	m_weaponOffset(weaponWielder.m_weaponOffset),
	m_activeWeaponIndex(weaponWielder.m_activeWeaponIndex),
	m_weaponByName(weaponWielder.m_weaponByName)
	{
		//FIXME:
		for (const Ndk::EntityHandle& entity : weaponWielder.m_weapons)
			m_weapons.emplace_back(entity);
	}
	
	inline const Ndk::EntityHandle& WeaponWielderComponent::GetActiveWeapon() const
	{
		return (m_activeWeaponIndex != NoWeapon) ? m_weapons[m_activeWeaponIndex] : Ndk::EntityHandle::InvalidHandle;
	}

	inline std::size_t WeaponWielderComponent::GetSelectedWeapon() const
	{
		return m_activeWeaponIndex;
	}

	inline const Ndk::EntityHandle& WeaponWielderComponent::GetWeapon(std::size_t weaponIndex) const
	{
		assert(weaponIndex < m_weapons.size());
		return m_weapons[weaponIndex];
	}

	inline const std::vector<Ndk::EntityOwner>& WeaponWielderComponent::GetWeapons() const
	{
		return m_weapons;
	}

	inline std::size_t WeaponWielderComponent::GetWeaponCount() const
	{
		return m_weapons.size();
	}

	inline const Nz::Vector2f& WeaponWielderComponent::GetWeaponOffset() const
	{
		return m_weaponOffset;
	}

	inline bool WeaponWielderComponent::HasActiveWeapon() const
	{
		return m_activeWeaponIndex != NoWeapon;
	}

	inline bool WeaponWielderComponent::HasWeapon(const std::string& weaponClass) const
	{
		return m_weaponByName.find(weaponClass) != m_weaponByName.end();
	}

	inline bool WeaponWielderComponent::SelectWeapon(const std::string& weaponClass)
	{
		auto it = m_weaponByName.find(weaponClass);
		if (it == m_weaponByName.end())
			return false;

		SelectWeapon(it->second);
		return true;
	}

	inline void WeaponWielderComponent::SetWeaponOffset(const Nz::Vector2f& weaponOffset)
	{
		m_weaponOffset = weaponOffset;
	}
}
