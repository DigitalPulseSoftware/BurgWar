// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_COMPONENTS_OWNER_HPP
#define BURGWAR_CORELIB_COMPONENTS_OWNER_HPP

#include <NDK/Component.hpp>
#include <CoreLib/Player.hpp>
#include <vector>

namespace bw
{
	class OwnerComponent : public Ndk::Component<OwnerComponent>
	{
		public:
			OwnerComponent(PlayerHandle owner);
			~OwnerComponent() = default;

			inline Player* GetOwner() const;

			static Ndk::ComponentIndex componentIndex;

		private:
			PlayerHandle m_owner;
	};
}

#include <CoreLib/Components/OwnerComponent.inl>

#endif
