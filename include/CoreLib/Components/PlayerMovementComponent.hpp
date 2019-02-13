// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_COMPONENTS_PLAYERMOVEMENT_HPP
#define BURGWAR_CORELIB_COMPONENTS_PLAYERMOVEMENT_HPP

#include <NDK/Component.hpp>
#include <vector>

namespace bw
{
	class PlayerMovementComponent : public Ndk::Component<PlayerMovementComponent>
	{
		public:
			PlayerMovementComponent();
			~PlayerMovementComponent() = default;

			inline float GetJumpTime() const;

			inline bool IsFacingRight() const;
			inline bool IsOnGround() const;

			inline bool UpdateFacingRightState(bool isFacingRight);
			inline void UpdateGroundState(bool isOnGround);
			inline void UpdateJumpTime(float jumpTime);
			inline void UpdateWasJumpingState(bool isJumping);

			inline bool WasJumping() const;

			static Ndk::ComponentIndex componentIndex;

		private:
			bool m_isFacingRight;
			bool m_isOnGround;
			bool m_lastJumpingState;
			float m_jumpTime;
	};
}

#include <CoreLib/Components/PlayerMovementComponent.inl>

#endif
