// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Components/WeaponWielderComponent.hpp>
#include <CoreLib/Components/ScriptComponent.hpp>
#include <CoreLib/Components/WeaponComponent.hpp>

namespace bw
{
	bool WeaponWielderComponent::GiveWeapon(std::string weaponClass, const WeaponInitCallback& callback)
	{
		if (HasWeapon(weaponClass))
			return false;

		Ndk::EntityHandle weapon = callback(weaponClass);
		if (!weapon)
			return false;

		//FIXME: New weapons should be resized to match the player size

		assert(weapon->HasComponent<WeaponComponent>());

		std::size_t weaponIndex = m_weapons.size();
		m_weapons.emplace_back(weapon);
		auto it = m_weaponByName.emplace(std::move(weaponClass), weaponIndex).first;

		OnWeaponAdded(this, it->first, weaponIndex);
		return true;
	}

	void WeaponWielderComponent::OverrideEntities(const std::function<void(Ndk::EntityOwner& owner)>& callback)
	{
		for (auto& weapon : m_weapons)
			callback(weapon);
	}

	void WeaponWielderComponent::RemoveWeapon(const std::string& weaponClass)
	{
		auto it = m_weaponByName.find(weaponClass);
		if (it == m_weaponByName.end())
			return;

		std::size_t droppedIndex = it->second;

		OnWeaponRemove(this, weaponClass, droppedIndex);

		if (m_activeWeaponIndex == droppedIndex)
			SelectWeapon(NoWeapon);

		m_weaponByName.erase(it);
		m_weapons.erase(m_weapons.begin() + droppedIndex);

		// Shift indexes by one
		for (auto weaponIt = m_weaponByName.begin(); weaponIt != m_weaponByName.end(); ++weaponIt)
		{
			std::size_t& weaponIndex = weaponIt.value();
			if (weaponIndex > droppedIndex)
				weaponIndex--;
		}
	}

	void WeaponWielderComponent::SelectWeapon(std::size_t weaponId)
	{
		assert(weaponId < m_weapons.size() || weaponId == NoWeapon);
		if (m_activeWeaponIndex != weaponId)
		{
			if (m_activeWeaponIndex != NoWeapon)
			{
				const Ndk::EntityHandle& previousWeapon = m_weapons[m_activeWeaponIndex];
				auto& weaponComponent = previousWeapon->GetComponent<WeaponComponent>();
				weaponComponent.SetActive(false);

				if (previousWeapon->HasComponent<ScriptComponent>())
				{
					auto& weaponScript = previousWeapon->GetComponent<ScriptComponent>();
					weaponScript.ExecuteCallback<ElementEvent::SwitchOff>();
				}
			}

			OnNewWeaponSelection(this, weaponId);
			m_activeWeaponIndex = weaponId;

			if (m_activeWeaponIndex != NoWeapon)
			{
				const Ndk::EntityHandle& newWeapon = m_weapons[m_activeWeaponIndex];
				auto& weaponComponent = newWeapon->GetComponent<WeaponComponent>();
				weaponComponent.SetActive(true);

				if (newWeapon->HasComponent<ScriptComponent>())
				{
					auto& weaponScript = newWeapon->GetComponent<ScriptComponent>();
					weaponScript.ExecuteCallback<ElementEvent::SwitchOn>();
				}
			}
		}
	}

	Ndk::ComponentIndex WeaponWielderComponent::componentIndex;
}
