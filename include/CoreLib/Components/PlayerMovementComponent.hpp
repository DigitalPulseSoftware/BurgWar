// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_COMPONENTS_PLAYERMOVEMENT_HPP
#define BURGWAR_CORELIB_COMPONENTS_PLAYERMOVEMENT_HPP

#include <NDK/Component.hpp>
#include <memory>
#include <vector>

namespace bw
{
	class PlayerMovementController;

	class PlayerMovementComponent : public Ndk::Component<PlayerMovementComponent>
	{
		public:
			PlayerMovementComponent();
			~PlayerMovementComponent() = default;

			inline const std::shared_ptr<PlayerMovementController>& GetController() const;
			inline float GetJumpTime() const;

			inline bool IsFacingRight() const;
			inline bool IsOnGround() const;

			void UpdateController(std::shared_ptr<PlayerMovementController> controller);
			inline bool UpdateFacingRightState(bool isFacingRight);
			inline void UpdateGroundState(bool isOnGround);
			inline void UpdateJumpTime(float jumpTime);
			inline void UpdateWasJumpingState(bool isJumping);

			inline bool WasJumping() const;

			static Ndk::ComponentIndex componentIndex;

		private:
			std::shared_ptr<PlayerMovementController> m_controller;
			bool m_isFacingRight;
			bool m_isOnGround;
			bool m_lastJumpingState;
			float m_jumpTime;
	};
}

#include <CoreLib/Components/PlayerMovementComponent.inl>

#endif
