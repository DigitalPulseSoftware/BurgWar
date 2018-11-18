// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/Components/PlayerControlledComponent.hpp>

namespace bw
{
	inline PlayerControlledComponent::PlayerControlledComponent() :
	m_isJumping(false),
	m_isMovingLeft(false),
	m_isMovingRight(false),
	m_isOnGround(false)
	{
	}

	inline bool PlayerControlledComponent::IsJumping() const
	{
		return m_isJumping;
	}

	inline bool PlayerControlledComponent::IsMovingLeft() const
	{
		return m_isMovingLeft;
	}

	inline bool PlayerControlledComponent::IsMovingRight() const
	{
		return m_isMovingRight;
	}

	inline bool PlayerControlledComponent::IsOnGround() const
	{
		return m_isOnGround;
	}

	inline void PlayerControlledComponent::UpdateJumpingState(bool isJumping)
	{
		m_isJumping = isJumping;
	}

	inline void PlayerControlledComponent::UpdateMovingLeftState(bool isMovingLeft)
	{
		m_isMovingLeft = isMovingLeft;
	}

	inline void PlayerControlledComponent::UpdateMovingRightState(bool isMovingRight)
	{
		m_isMovingRight = isMovingRight;
	}

	inline bool PlayerControlledComponent::UpdateGroundState(bool isOnGround)
	{
		if (m_isOnGround == isOnGround)
			return false;

		m_isOnGround = isOnGround;
		return true;
	}
}
