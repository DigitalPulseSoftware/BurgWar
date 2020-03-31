// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_COMPONENTS_WEAPONWIELDERCOMPONENT_HPP
#define BURGWAR_CORELIB_COMPONENTS_WEAPONWIELDERCOMPONENT_HPP

#include <Nazara/Math/Vector2.hpp>
#include <NDK/Component.hpp>

namespace bw
{
	class WeaponWielderComponent : public Ndk::Component<WeaponWielderComponent>
	{
		public:
			inline WeaponWielderComponent();
			~WeaponWielderComponent() = default;

			inline const Nz::Vector2f& GetWeaponOffset() const;

			inline void SetWeaponOffset(const Nz::Vector2f& weaponOffset);

			static Ndk::ComponentIndex componentIndex;

		private:
			Nz::Vector2f m_weaponOffset;
	};
}

#include <CoreLib/Components/WeaponWielderComponent.inl>

#endif
