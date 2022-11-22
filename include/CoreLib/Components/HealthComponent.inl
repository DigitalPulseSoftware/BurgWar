// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Components/HealthComponent.hpp>

namespace bw
{
	inline HealthComponent::HealthComponent(entt::handle entity, Nz::UInt16 maxHealth) :
	BaseComponent(entity),
	m_currentHealth(maxHealth),
	m_maxHealth(maxHealth)
	{
	}

	inline HealthComponent::HealthComponent(const HealthComponent& health) :
	BaseComponent(health),
	m_currentHealth(health.m_currentHealth),
	m_maxHealth(health.m_maxHealth)
	{
	}

	inline void HealthComponent::Damage(Nz::UInt16 damage, entt::handle attacker)
	{
		OnDamage(this, damage, attacker);

		Nz::UInt16 newHealth = m_currentHealth;
		if (damage >= newHealth)
			newHealth = 0;
		else
			newHealth -= damage;

		if (m_currentHealth != newHealth)
		{
			OnHealthChange(this, newHealth, attacker);

			m_currentHealth = newHealth;

			if (m_currentHealth == 0)
			{
				OnDying(this, attacker);

				// Recheck health in case a slot gave us health
				if (m_currentHealth == 0)
					OnDie(this, attacker);
			}
		}
	}

	inline Nz::UInt16 HealthComponent::GetHealth() const
	{
		return m_currentHealth;
	}

	inline float HealthComponent::GetHealthPct() const
	{
		return 100.f * m_currentHealth / m_maxHealth;
	}

	inline Nz::UInt16 HealthComponent::GetMaxHealth() const
	{
		return m_maxHealth;
	}

	inline void HealthComponent::Heal(Nz::UInt16 heal, entt::handle healer)
	{
		Nz::UInt16 newHealth = m_currentHealth + heal;
		if (newHealth < m_currentHealth)
			// Overflow, just set to max value
			newHealth = m_maxHealth;

		if (newHealth > m_maxHealth)
			newHealth = m_maxHealth;

		if (m_currentHealth != newHealth)
		{
			OnHealthChange(this, newHealth, healer);
			m_currentHealth = newHealth;
		}
	}
}
