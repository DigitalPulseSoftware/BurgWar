// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_BASICPLAYERMOVEMENTCONTROLLER_HPP
#define BURGWAR_CORELIB_BASICPLAYERMOVEMENTCONTROLLER_HPP

#include <CoreLib/Export.hpp>
#include <CoreLib/PlayerMovementController.hpp>

namespace bw
{
	class BURGWAR_CORELIB_API BasicPlayerMovementController : public PlayerMovementController
	{
		public:
			BasicPlayerMovementController() = default;
			~BasicPlayerMovementController() = default;

			bool PreSolveCollision(PlayerMovementComponent& playerMovement, entt::entity collisionBody, Nz::PhysArbiter2D& arbiter) const override;

			void UpdateVelocity(const PlayerInputData& inputs, PlayerMovementComponent& playerMovement, Nz::RigidBody2D& rigidBody, const Nz::Vector2f& gravity, float damping, float dt) const override;
	};
}

#include <CoreLib/BasicPlayerMovementController.inl>

#endif
