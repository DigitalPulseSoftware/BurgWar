// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Systems/PlayerMovementSystem.hpp>
#include <Nazara/Physics2D/Arbiter2D.hpp>
#include <NDK/Components.hpp>
#include <CoreLib/PlayerMovementController.hpp>
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

	void PlayerMovementSystem::OnUpdate(float /*elapsedTime*/)
	{
		for (const Ndk::EntityHandle& entity : GetEntities())
		{
			auto& inputComponent = entity->GetComponent<InputComponent>();
			auto& playerMovementComponent = entity->GetComponent<PlayerMovementComponent>();
			auto& nodeComponent = entity->GetComponent<Ndk::NodeComponent>();

			const auto& inputs = inputComponent.GetInputs();

			playerMovementComponent.UpdateWasJumpingState(inputs.isJumping);

			if (playerMovementComponent.UpdateFacingRightState(inputs.isLookingRight))
				nodeComponent.Scale(-1.f, 1.f);
		}
	}

	Ndk::SystemIndex PlayerMovementSystem::systemIndex;
}
