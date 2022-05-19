// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Components/WeaponWielderComponent.hpp>

namespace bw
{
	inline WeaponWielderComponent::WeaponWielderComponent(entt::handle entity) :
	BaseComponent(entity),
	m_activeWeaponIndex(NoWeapon),
	m_weaponOffset(Nz::Vector2f::Zero())
	{
	}

	inline WeaponWielderComponent::WeaponWielderComponent(const WeaponWielderComponent& weaponWielder) :
	BaseComponent(weaponWielder),
	m_activeWeaponIndex(weaponWielder.m_activeWeaponIndex),
	m_weaponByName(weaponWielder.m_weaponByName),
	m_weaponOffset(weaponWielder.m_weaponOffset)
	{
		//FIXME:
		for (entt::handle entity : weaponWielder.m_weapons)
			m_weapons.emplace_back(entity);
	}
	
	inline entt::handle WeaponWielderComponent::GetActiveWeapon() const
	{
		return (m_activeWeaponIndex != NoWeapon) ? m_weapons[m_activeWeaponIndex].GetEntity() : entt::handle{};
	}

	inline std::size_t WeaponWielderComponent::GetSelectedWeapon() const
	{
		return m_activeWeaponIndex;
	}

	inline entt::handle WeaponWielderComponent::GetWeapon(std::size_t weaponIndex) const
	{
		assert(weaponIndex < m_weapons.size());
		return m_weapons[weaponIndex];
	}

	inline const std::vector<EntityOwner>& WeaponWielderComponent::GetWeapons() const
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
