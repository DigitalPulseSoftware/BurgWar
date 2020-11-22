// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Components/PlayerMovementComponent.hpp>

namespace bw
{
	inline PlayerMovementComponent::PlayerMovementComponent() :
	m_targetVelocity(Nz::Vector2f::Zero()),
	m_isFacingRight(true),
	m_isOnGround(false),
	m_lastJumpingState(false),
	m_groundFriction(0.f),
	m_jumpBoostHeight(80.f),
	m_jumpHeight(80.f),
	m_jumpTime(0.f),
	m_movementSpeed(500.f)
	{
	}

	inline const std::shared_ptr<PlayerMovementController>& PlayerMovementComponent::GetController() const
	{
		return m_controller;
	}

	inline float PlayerMovementComponent::GetGroundFriction() const
	{
		return m_groundFriction;
	}

	inline float PlayerMovementComponent::GetJumpBoostHeight() const
	{
		return m_jumpBoostHeight;
	}

	inline float PlayerMovementComponent::GetJumpHeight() const
	{
		return m_jumpHeight;
	}

	inline float PlayerMovementComponent::GetJumpTime() const
	{
		return m_jumpTime;
	}

	inline float PlayerMovementComponent::GetMovementSpeed() const
	{
		return m_movementSpeed;
	}

	inline const Nz::Vector2f& PlayerMovementComponent::GetTargetVelocity() const
	{
		return m_targetVelocity;
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

	inline void PlayerMovementComponent::UpdateGroundFriction(float groundFriction)
	{
		m_groundFriction = groundFriction;
	}

	inline void PlayerMovementComponent::UpdateGroundState(bool isOnGround)
	{
		m_isOnGround = isOnGround;
	}

	inline void PlayerMovementComponent::UpdateJumpBoostHeight(float boostHeight)
	{
		m_jumpBoostHeight = boostHeight;
	}

	inline void PlayerMovementComponent::UpdateJumpHeight(float jumpHeight)
	{
		m_jumpHeight = jumpHeight;
	}

	inline void PlayerMovementComponent::UpdateJumpTime(float jumpTime)
	{
		m_jumpTime = jumpTime;
	}

	inline void PlayerMovementComponent::UpdateMovementSpeed(float movementSpeed)
	{
		m_movementSpeed = movementSpeed;
	}

	inline void PlayerMovementComponent::UpdateTargetVelocity(const Nz::Vector2f& targetVelocity)
	{
		m_targetVelocity = targetVelocity;
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
