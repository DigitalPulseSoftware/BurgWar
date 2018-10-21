// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/Systems/PlayerMovementSystem.hpp>
#include <NDK/Components.hpp>
#include <Shared/Components/PlayerMovementComponent.hpp>

namespace bw
{
	PlayerMovementSystem::PlayerMovementSystem()
	{
		Requires<PlayerMovementComponent, Ndk::PhysicsComponent2D>();
	}

	void PlayerMovementSystem::OnUpdate(float elapsedTime)
	{
		for (const Ndk::EntityHandle& entity : GetEntities())
		{
			auto& playerMovementComponent = entity->GetComponent<PlayerMovementComponent>();
			auto& physicsComponent = entity->GetComponent<Ndk::PhysicsComponent2D>();

			if (playerMovementComponent.IsAirControlling())
				physicsComponent.AddImpulse(Nz::Vector2f(-physicsComponent.GetVelocity().x / 2.f, 0.f) * physicsComponent.GetMass());
		}
	}

	Ndk::SystemIndex PlayerMovementSystem::systemIndex;
}
