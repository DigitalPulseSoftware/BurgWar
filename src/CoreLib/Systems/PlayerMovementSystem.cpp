// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Systems/PlayerMovementSystem.hpp>
#include <Nazara/Physics2D/Arbiter2D.hpp>
#include <NDK/Components.hpp>
#include <CoreLib/Components/InputComponent.hpp>
#include <CoreLib/Components/PlayerMovementComponent.hpp>

namespace bw
{
	PlayerMovementSystem::PlayerMovementSystem()
	{
		Requires<InputComponent, PlayerMovementComponent, Ndk::PhysicsComponent2D>();
	}
	
	void PlayerMovementSystem::OnEntityAdded(Ndk::Entity* entity)
	{
		Ndk::PhysicsComponent2D& entityPhys = entity->GetComponent<Ndk::PhysicsComponent2D>();
		entityPhys.SetVelocityFunction([entity = entity->CreateHandle()](Nz::RigidBody2D& rigidBody, const Nz::Vector2f& gravity, float damping, float dt)
		{
			auto& inputComponent = entity->GetComponent<InputComponent>();
			auto& movementComponent = entity->GetComponent<PlayerMovementComponent>();

			const auto& inputs = inputComponent.GetInputData();

			Nz::Vector2f up = Nz::Vector2f::UnitY();

			bool isOnGround = false;
			rigidBody.ForEachArbiter([&](Nz::Arbiter2D& arbiter)
			{
				if (up.DotProduct(arbiter.GetNormal()) > 0.75f)
					isOnGround = true;
			});

			movementComponent.UpdateGroundState(isOnGround);

			bool disableGravity = false;
			if (inputs.isJumping)
			{
				float jumpTime = movementComponent.GetJumpTime();
				if (jumpTime > 0.f)
				{
					disableGravity = true;

					movementComponent.UpdateJumpTime(jumpTime - dt);
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

			rigidBody.SetSurfaceVelocity(Nz::Vector2f(-targetVelocity, 0.f));
			rigidBody.SetFriction((isOnGround) ? groundAccel / gravity.y : 0.f);

			if (!isOnGround)
			{
				Nz::Vector2f velocity = rigidBody.GetVelocity();
				velocity.x = Nz::Approach(velocity.x, targetVelocity, airAccel * dt);
				rigidBody.SetVelocity(velocity);
			}
		});
	}

	void PlayerMovementSystem::OnEntityRemoved(Ndk::Entity* entity)
	{
		if (!entity->HasComponent<Ndk::PhysicsComponent2D>())
			return;

		Ndk::PhysicsComponent2D& entityPhys = entity->GetComponent<Ndk::PhysicsComponent2D>();
		if (!entityPhys.IsValid())
			return;

		entityPhys.ResetVelocityFunction();
	}

	void PlayerMovementSystem::OnUpdate(float elapsedTime)
	{
		for (const Ndk::EntityHandle& entity : GetEntities())
		{
			auto& inputComponent = entity->GetComponent<InputComponent>();
			auto& playerMovementComponent = entity->GetComponent<PlayerMovementComponent>();
			auto& nodeComponent = entity->GetComponent<Ndk::NodeComponent>();
			auto& physicsComponent = entity->GetComponent<Ndk::PhysicsComponent2D>();

			const auto& inputs = inputComponent.GetInputData();

			if (inputs.isJumping && !playerMovementComponent.WasJumping() && playerMovementComponent.IsOnGround())
			{
				constexpr float jumpHeight = 50.f;
				constexpr float jumpBoostHeight = 55.f;

				float jumpVelocity = std::sqrt(2.f * jumpHeight * 9.81f * 128.f);
				physicsComponent.SetVelocity(physicsComponent.GetVelocity() + Nz::Vector2f(0.f, -jumpVelocity));

				playerMovementComponent.UpdateJumpTime(jumpBoostHeight / jumpVelocity);
			}

			bool isFacingRight = playerMovementComponent.IsFacingRight();

			if (inputs.isMovingLeft)
				isFacingRight = false;
			else if (inputs.isMovingRight)
				isFacingRight = true;
			else if (inputs.aimDirection.x > 0.f)
				isFacingRight = true;
			else if (inputs.aimDirection.x < 0.f)
				isFacingRight = false;

			playerMovementComponent.UpdateWasJumpingState(inputs.isJumping);

			if (playerMovementComponent.UpdateFacingRightState(isFacingRight))
				nodeComponent.Scale(-1.f, 1.f);
		}
	}

	Ndk::SystemIndex PlayerMovementSystem::systemIndex;
}
