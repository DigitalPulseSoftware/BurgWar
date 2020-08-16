// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/BasicPlayerMovementController.hpp>
#include <CoreLib/PlayerInputData.hpp>
#include <CoreLib/Components/PlayerMovementComponent.hpp>
#include <Nazara/Physics2D/Arbiter2D.hpp>
#include <Nazara/Physics2D/RigidBody2D.hpp>

namespace bw
{
	void BasicPlayerMovementController::UpdateVelocity(const PlayerInputData& inputs, PlayerMovementComponent& playerMovement, Nz::RigidBody2D& rigidBody, const Nz::Vector2f& gravity, float damping, float dt) const
	{
		bool isOnGround = playerMovement.IsOnGround();
		bool disableGravity = false;
		float jumpVelocity = 0.f;

		if (inputs.isJumping)
		{
			if (!playerMovement.WasJumping() && isOnGround)
			{
				float jumpHeight = playerMovement.GetJumpHeight();
				float jumpBoostHeight = playerMovement.GetJumpBoostHeight();

				jumpVelocity = std::sqrt(2.f * jumpHeight * 9.81f * 128.f);

				playerMovement.UpdateJumpTime(jumpBoostHeight / jumpVelocity);
			}

			float jumpTime = playerMovement.GetJumpTime();
			if (jumpTime > 0.f)
			{
				disableGravity = true;

				playerMovement.UpdateJumpTime(jumpTime - dt);
			}
		}

		rigidBody.UpdateVelocity((disableGravity) ? Nz::Vector2f::Zero() : gravity, damping, dt);

		float playerVelocity = playerMovement.GetMovementSpeed();

		constexpr float groundAccelTime = 0.1f;
		float groundAccel = playerVelocity / groundAccelTime;

		constexpr float airAccelTime = 0.5f;
		float airAccel = playerVelocity / airAccelTime;

		float targetVelocity = 0.f;
		if (inputs.isMovingLeft)
			targetVelocity -= playerVelocity;

		if (inputs.isMovingRight)
			targetVelocity += playerVelocity;

		rigidBody.SetSurfaceVelocity(0, Nz::Vector2f(-targetVelocity, 0.f));
		rigidBody.SetFriction(0, (isOnGround) ? groundAccel / gravity.y : 0.f);

		Nz::Vector2f velocity = rigidBody.GetVelocity();

		velocity.y -= jumpVelocity;

		// Air control
		if (!isOnGround)
			velocity.x = Nz::Approach(velocity.x, targetVelocity, airAccel * dt);

		rigidBody.SetVelocity(velocity);
	}
}
