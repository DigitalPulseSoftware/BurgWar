// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/Components/PlayerMovementComponent.hpp>

namespace bw
{
	inline PlayerMovementComponent::PlayerMovementComponent() :
	m_isAirControlling(false),
	m_isFacingRight(true),
	m_isOnGround(false),
	m_lastJumpingState(false),
	m_jumpTime(0.f)
	{
	}

	inline float PlayerMovementComponent::GetJumpTime() const
	{
		return m_jumpTime;
	}

	inline bool PlayerMovementComponent::IsAirControlling() const
	{
		return m_isAirControlling;
	}

	inline bool PlayerMovementComponent::IsFacingRight() const
	{
		return m_isFacingRight;
	}

	inline bool PlayerMovementComponent::IsOnGround() const
	{
		return m_isOnGround;
	}

	inline bool PlayerMovementComponent::UpdateAirControlState(bool isAirControlling)
	{
		if (m_isAirControlling == isAirControlling)
			return false;

		m_isAirControlling = isAirControlling;
		return true;
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
