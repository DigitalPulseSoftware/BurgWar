// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_SHARED_COMPONENTS_PLAYERCONTROLLED_HPP
#define BURGWAR_SHARED_COMPONENTS_PLAYERCONTROLLED_HPP

#include <NDK/Component.hpp>
#include <Shared/Player.hpp>
#include <vector>

namespace bw
{
	class PlayerControlledComponent : public Ndk::Component<PlayerControlledComponent>
	{
		public:
			PlayerControlledComponent(PlayerHandle owner);
			~PlayerControlledComponent() = default;

			inline Player* GetOwner() const;

			static Ndk::ComponentIndex componentIndex;

		private:
			PlayerHandle m_owner;
	};
}

#include <Shared/Components/PlayerControlledComponent.inl>

#endif
