// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/Systems/PlayerControlledSystem.hpp>
#include <Nazara/Physics2D/Arbiter2D.hpp>
#include <NDK/Components.hpp>
#include <Shared/Components/PlayerControlledComponent.hpp>
#include <Shared/Components/PlayerMovementComponent.hpp>

namespace bw
{
	PlayerControlledSystem::PlayerControlledSystem()
	{
		Requires<PlayerControlledComponent, PlayerMovementComponent, Ndk::NodeComponent, Ndk::PhysicsComponent2D>();
	}

	void PlayerControlledSystem::OnUpdate(float elapsedTime)
	{
		constexpr float jumpForce = 500.f;
		constexpr float bumpForceX = 500.f;
		constexpr float bumpForceY = 200.f;

		for (const Ndk::EntityHandle& entity : GetEntities())
		{
			auto& playerControlledComponent = entity->GetComponent<PlayerControlledComponent>();
			auto& playerMovementComponent = entity->GetComponent<PlayerMovementComponent>();
			auto& nodeComponent = entity->GetComponent<Ndk::NodeComponent>();
			auto& physicsComponent = entity->GetComponent<Ndk::PhysicsComponent2D>();

			bool isFacingRight = playerMovementComponent.IsFacingRight();

			Nz::Vector2f up = Nz::Vector2f::UnitY();

			bool isOnGround = false;
			physicsComponent.ForEachArbiter([&](Nz::Arbiter2D& arbiter)
			{
				if (up.DotProduct(arbiter.GetNormal()) > 0.75f)
					isOnGround = true;
			});

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

			playerMovementComponent.UpdateAirControlState(!isMovementKeyPressed && !isOnGround);
			if (playerMovementComponent.UpdateFacingRightState(isFacingRight))
				nodeComponent.Scale(-1.f, 1.f);
		}
	}

	Ndk::SystemIndex PlayerControlledSystem::systemIndex;
}
