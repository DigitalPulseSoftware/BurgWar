// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Systems/PlayerMovementSystem.hpp>
#include <Nazara/Physics2D/Arbiter2D.hpp>
#include <CoreLib/PlayerMovementController.hpp>
#include <CoreLib/Components/InputComponent.hpp>
#include <CoreLib/Components/PlayerMovementComponent.hpp>

namespace bw
{
	PlayerMovementSystem::PlayerMovementSystem(entt::registry& registry)
	{
		//Requires<InputComponent, PlayerMovementComponent, Ndk::PhysicsComponent2D>();
		//SetUpdateOrder(50); //< Execute after physics but before rendering
	}
	
	void PlayerMovementSystem::OnEntityAdded(Ndk::Entity* entity)
	{
		/*Ndk::PhysicsComponent2D& entityPhys = entity->GetComponent<Ndk::PhysicsComponent2D>();
		entityPhys.SetVelocityFunction([entity = entity->CreateHandle()](Nz::RigidBody2D& rigidBody, const Nz::Vector2f& gravity, float damping, float dt)
		{
			auto& movementComponent = entity->GetComponent<PlayerMovementComponent>();

			const auto& controller = movementComponent.GetController();
			if (controller)
			{
				auto& inputComponent = entity->GetComponent<InputComponent>();
				const auto& inputs = inputComponent.GetInputs();

				controller->UpdateVelocity(inputs, movementComponent, rigidBody, gravity, damping, dt);
			}
			else
				rigidBody.UpdateVelocity(gravity, damping, dt);
		});*/
	}

	void PlayerMovementSystem::OnEntityRemoved(Ndk::Entity* entity)
	{
		/*if (!entity->HasComponent<Ndk::PhysicsComponent2D>())
			return;

		Ndk::PhysicsComponent2D& entityPhys = entity->GetComponent<Ndk::PhysicsComponent2D>();
		if (!entityPhys.IsValid())
			return;

		entityPhys.ResetVelocityFunction();*/
	}

	void PlayerMovementSystem::OnUpdate(float /*elapsedTime*/)
	{
		/*for (entt::entity entity : GetEntities())
		{
			auto& inputComponent = entity->GetComponent<InputComponent>();
			auto& playerMovement = entity->GetComponent<PlayerMovementComponent>();
			auto& nodeComponent = entity->GetComponent<Ndk::NodeComponent>();
			auto& entityPhys = entity->GetComponent<Ndk::PhysicsComponent2D>();

			const auto& inputs = inputComponent.GetInputs();
			
			Nz::Vector2f up = Nz::Vector2f::UnitY();

			bool isOnGround = false;
			entityPhys.ForEachArbiter([&](Nz::Arbiter2D& arbiter)
			{
				if (up.DotProduct(arbiter.GetNormal()) > 0.75f)
					isOnGround = true;
			});

			playerMovement.UpdateGroundState(isOnGround);

			playerMovement.UpdateWasJumpingState(inputs.isJumping);

			if (playerMovement.UpdateFacingRightState(inputs.isLookingRight))
				nodeComponent.Scale(-1.f, 1.f);
		}*/
	}
}
