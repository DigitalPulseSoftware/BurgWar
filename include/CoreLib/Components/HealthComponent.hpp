// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_COMPONENTS_HEALTHCOMPONENT_HPP
#define BURGWAR_CORELIB_COMPONENTS_HEALTHCOMPONENT_HPP

#include <CoreLib/Export.hpp>
#include <CoreLib/Components/BaseComponent.hpp>
#include <Nazara/Utils/Signal.hpp>

namespace bw
{
	class BURGWAR_CORELIB_API HealthComponent : public BaseComponent
	{
		public:
			inline HealthComponent(entt::handle entity, Nz::UInt16 maxHealth);
			inline HealthComponent(const HealthComponent& health);

			inline void Damage(Nz::UInt16 damage, entt::handle attacker);

			inline Nz::UInt16 GetHealth() const;
			inline float GetHealthPct() const;
			inline Nz::UInt16 GetMaxHealth() const;

			inline void Heal(Nz::UInt16 heal, entt::handle healer);

			NazaraSignal(OnDamage, HealthComponent* /*emitter*/, Nz::UInt16& /*damage*/, entt::handle /*attacker*/);
			NazaraSignal(OnDying, HealthComponent* /*emitter*/, entt::handle /*attacker*/);
			NazaraSignal(OnDie, const HealthComponent* /*emitter*/, entt::handle /*attacker*/);
			NazaraSignal(OnHealthChange, HealthComponent* /*emitter*/, Nz::UInt16 /*newHealth*/, entt::handle /*source*/);

		private:
			Nz::UInt16 m_currentHealth;
			Nz::UInt16 m_maxHealth;
	};
}

#include <CoreLib/Components/HealthComponent.inl>

#endif
