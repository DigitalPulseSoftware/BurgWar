// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_COMPONENTS_WEAPONCOMPONENT_HPP
#define BURGWAR_CORELIB_COMPONENTS_WEAPONCOMPONENT_HPP

#include <NDK/Component.hpp>

namespace bw
{
	class WeaponComponent : public Ndk::Component<WeaponComponent>
	{
		public:
			WeaponComponent(Ndk::EntityHandle owner);
			~WeaponComponent() = default;

			inline const Ndk::EntityHandle& GetOwner() const;

			static Ndk::ComponentIndex componentIndex;

		private:
			Ndk::EntityHandle m_owner;
	};
}

#include <CoreLib/Components/WeaponComponent.inl>

#endif
