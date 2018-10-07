// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/Systems/PlayerControlledSystem.hpp>
#include <NDK/Components.hpp>
#include <Client/Components/PlayerControlledComponent.hpp>

namespace bw
{
	PlayerControlledSystem::PlayerControlledSystem()
	{
		Requires<PlayerControlledComponent, Ndk::NodeComponent, Ndk::PhysicsComponent2D>();
	}

	void PlayerControlledSystem::OnUpdate(float elapsedTime)
	{
		constexpr float jumpForce = 500.f;
		constexpr float bumpForceX = 500.f;
		constexpr float bumpForceY = 200.f;

		for (const Ndk::EntityHandle& entity : GetEntities())
		{
			auto& playerControlledComponent = entity->GetComponent<PlayerControlledComponent>();
			auto& nodeComponent = entity->GetComponent<Ndk::NodeComponent>();
			auto& physicsComponent = entity->GetComponent<Ndk::PhysicsComponent2D>();

			bool isFacingRight = playerControlledComponent.IsFacingRight();
			bool isOnGround = playerControlledComponent.IsOnGround();

			bool isJumping = playerControlledComponent.IsJumping();
			bool isMovingLeft = playerControlledComponent.IsMovingLeft();
			bool isMovingRight = playerControlledComponent.IsMovingRight();

			if (isJumping && isOnGround)
			{
				float dirForce = 0.f;
				if (isMovingLeft)
					dirForce = -bumpForceX;
				else if (isMovingRight)
					dirForce = bumpForceX;

				physicsComponent.AddImpulse(Nz::Vector2f(dirForce, -jumpForce) * physicsComponent.GetMass());

				isOnGround = false;
			}

			bool isMovementKeyPressed = false;
			if (isMovingLeft)
			{
				isMovementKeyPressed = true;

				if (isOnGround)
				{
					physicsComponent.AddImpulse(Nz::Vector2f(-bumpForceX, -bumpForceY) * physicsComponent.GetMass());

					isFacingRight = false;
					isOnGround = false;
				}
			}

			if (isMovingRight)
			{
				isMovementKeyPressed = true;

				if (isOnGround)
				{
					physicsComponent.AddImpulse(Nz::Vector2f(bumpForceX, -bumpForceY) * physicsComponent.GetMass());

					isFacingRight = true;
					isOnGround = false;
				}
			}

			if (!isMovementKeyPressed && !isOnGround)
				physicsComponent.AddImpulse(Nz::Vector2f(-physicsComponent.GetVelocity().x / 2.f, 0.f) * physicsComponent.GetMass());

			if (playerControlledComponent.UpdateFacingRightState(isFacingRight))
				nodeComponent.Scale(-1.f, 1.f);

			playerControlledComponent.UpdateGroundState(isOnGround);
		}
	}

	Ndk::SystemIndex PlayerControlledSystem::systemIndex;
}
