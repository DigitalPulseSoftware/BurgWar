// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENT_COMPONENTS_PLAYERCONTROLLED_HPP
#define BURGWAR_CLIENT_COMPONENTS_PLAYERCONTROLLED_HPP

#include <NDK/Component.hpp>
#include <vector>

namespace bw
{
	class PlayerControlledComponent : public Ndk::Component<PlayerControlledComponent>
	{
		friend class PlayerControlledSystem;
		friend class TerrainLayer;

		public:
			PlayerControlledComponent();
			~PlayerControlledComponent() = default;

			bool IsJumping() const;
			bool IsMovingLeft() const;
			bool IsMovingRight() const;
			inline bool IsOnGround() const;

			static Ndk::ComponentIndex componentIndex;

		private:
			inline bool UpdateGroundState(bool isOnGround);

			bool m_isOnGround;
	};
}

#include <Shared/Components/PlayerControlledComponent.inl>

#endif
