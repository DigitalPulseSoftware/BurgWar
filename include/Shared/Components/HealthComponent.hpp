// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_SHARED_COMPONENTS_HEALTHCOMPONENT_HPP
#define BURGWAR_SHARED_COMPONENTS_HEALTHCOMPONENT_HPP

#include <Nazara/Core/Signal.hpp>
#include <NDK/Component.hpp>

namespace bw
{
	class HealthComponent : public Ndk::Component<HealthComponent>
	{
		public:
			inline HealthComponent(Nz::UInt16 maxHealth);
			inline HealthComponent(const HealthComponent&);

			inline void Damage(Nz::UInt16 damage, const Ndk::EntityHandle& attacker);

			inline Nz::UInt16 GetHealth() const;
			inline float GetHealthPct() const;
			inline Nz::UInt16 GetMaxHealth() const;

			inline void Heal(Nz::UInt16 heal);

			static Ndk::ComponentIndex componentIndex;

			NazaraSignal(OnDying, HealthComponent* /*emitter*/, const Ndk::EntityHandle& /*attacker*/);
			NazaraSignal(OnDied, const HealthComponent* /*emitter*/, const Ndk::EntityHandle& /*attacker*/);
			NazaraSignal(OnHealthChange, HealthComponent* /*emitter*/);

		private:
			Nz::UInt16 m_currentHealth;
			Nz::UInt16 m_maxHealth;
	};
}

#include <Shared/Components/HealthComponent.inl>

#endif
