// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Components/PlayerMovementComponent.hpp>

namespace bw
{
	inline PlayerMovementComponent::PlayerMovementComponent() :
	m_isFacingRight(true),
	m_isOnGround(false),
	m_lastJumpingState(false),
	m_jumpTime(0.f)
	{
	}

	inline const std::shared_ptr<PlayerMovementController>& PlayerMovementComponent::GetController() const
	{
		return m_controller;
	}

	inline float PlayerMovementComponent::GetJumpTime() const
	{
		return m_jumpTime;
	}

	inline bool PlayerMovementComponent::IsFacingRight() const
	{
		return m_isFacingRight;
	}

	inline bool PlayerMovementComponent::IsOnGround() const
	{
		return m_isOnGround;
	}

	inline void PlayerMovementComponent::UpdateController(std::shared_ptr<PlayerMovementController> controller)
	{
		m_controller = std::move(controller);
	}

	inline bool PlayerMovementComponent::UpdateFacingRightState(bool isFacingRight)
	{
		if (m_isFacingRight == isFacingRight)
			return false;

		m_isFacingRight = isFacingRight;
		return true;
	}

	inline void PlayerMovementComponent::UpdateGroundState(bool isOnGround)
	{
		m_isOnGround = isOnGround;
	}

	inline void PlayerMovementComponent::UpdateJumpTime(float jumpTime)
	{
		m_jumpTime = jumpTime;
	}

	inline void PlayerMovementComponent::UpdateWasJumpingState(bool isJumping)
	{
		m_lastJumpingState = isJumping;
	}

	inline bool PlayerMovementComponent::WasJumping() const
	{
		return m_lastJumpingState;
	}
}
