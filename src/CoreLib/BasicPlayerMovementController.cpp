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
		Nz::Vector2f up = Nz::Vector2f::UnitY();

		bool isOnGround = false;
		rigidBody.ForEachArbiter([&](Nz::Arbiter2D& arbiter)
		{
			if (up.DotProduct(arbiter.GetNormal()) > 0.75f)
				isOnGround = true;
		});

		playerMovement.UpdateGroundState(isOnGround);

		bool disableGravity = false;
		float jumpVelocity = 0.f;

		if (inputs.isJumping)
		{
			if (!playerMovement.WasJumping() && playerMovement.IsOnGround())
			{
				constexpr float jumpHeight = 80.f;
				constexpr float jumpBoostHeight = 55.f;

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

		constexpr float playerVelocity = 500.f;

		constexpr float groundAccelTime = 0.1f;
		constexpr float groundAccel = playerVelocity / groundAccelTime;

		constexpr float airAccelTime = 0.5f;
		constexpr float airAccel = playerVelocity / airAccelTime;

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
