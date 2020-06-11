// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/NoclipPlayerMovementController.hpp>
#include <CoreLib/PlayerInputData.hpp>
#include <CoreLib/Components/PlayerMovementComponent.hpp>
#include <Nazara/Physics2D/Arbiter2D.hpp>
#include <Nazara/Physics2D/RigidBody2D.hpp>

namespace bw
{
	void NoclipPlayerMovementController::UpdateVelocity(const PlayerInputData& inputs, PlayerMovementComponent& /*playerMovement*/, Nz::RigidBody2D& rigidBody, const Nz::Vector2f& /*gravity*/, float damping, float dt) const
	{
		constexpr float noclipVelocity = 2000.f;
		constexpr float noclipAccelTime = 0.2f;
		constexpr float noclipAccel = noclipVelocity / noclipAccelTime;

		Nz::Vector2f targetVelocity = Nz::Vector2f::Zero();
		if (inputs.isCrouching)
			targetVelocity.y += noclipVelocity;

		if (inputs.isJumping)
			targetVelocity.y -= noclipVelocity;

		if (inputs.isMovingLeft)
			targetVelocity.x -= noclipVelocity;

		if (inputs.isMovingRight)
			targetVelocity.x += noclipVelocity;

		Nz::Vector2f currentVelocity = rigidBody.GetVelocity();
		currentVelocity.x = Nz::Approach(currentVelocity.x, targetVelocity.x, noclipAccel * dt);
		currentVelocity.y = Nz::Approach(currentVelocity.y, targetVelocity.y, noclipAccel * dt);
		rigidBody.SetVelocity(currentVelocity);

		rigidBody.UpdateVelocity(Nz::Vector2f::Zero(), damping, dt);
	}
}
