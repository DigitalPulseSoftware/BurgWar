// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_COMPONENTS_WEAPONCOMPONENT_HPP
#define BURGWAR_CORELIB_COMPONENTS_WEAPONCOMPONENT_HPP

#include <NDK/Component.hpp>

namespace bw
{
	enum class WeaponAttackMode
	{
		SingleShot,
		SingleShotRepeat,

		Max = SingleShotRepeat
	};

	class WeaponComponent : public Ndk::Component<WeaponComponent>
	{
		public:
			inline WeaponComponent(Ndk::EntityHandle owner, WeaponAttackMode attackMode);
			~WeaponComponent() = default;

			inline WeaponAttackMode GetAttackMode() const;
			inline const Ndk::EntityHandle& GetOwner() const;

			inline bool IsActive() const;
			inline bool IsAttacking() const;

			inline void SetActive(bool isActive);
			inline void SetAttacking(bool isAttacking);

			inline void UpdateOwner(Ndk::EntityHandle owner);

			static Ndk::ComponentIndex componentIndex;

		private:
			Ndk::EntityHandle m_owner;
			WeaponAttackMode m_attackMode;
			bool m_isActive;
			bool m_isAttacking;
	};
}

#include <CoreLib/Components/WeaponComponent.inl>

#endif
