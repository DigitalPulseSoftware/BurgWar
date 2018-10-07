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

			inline bool IsFacingRight() const;

			bool IsMovingLeft() const;
			bool IsMovingRight() const;

			bool IsJumping() const;

			inline bool IsOnGround() const;

			static Ndk::ComponentIndex componentIndex;

		private:
			inline bool UpdateFacingRightState(bool isFacingRight);
			inline bool UpdateGroundState(bool isOnGround);

			bool m_isFacingRight;
			bool m_isOnGround;
	};
}

#include <Client/Components/PlayerControlledComponent.inl>

#endif
