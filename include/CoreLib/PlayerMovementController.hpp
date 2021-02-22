// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_PLAYERMOVEMENTCONTROLLER_HPP
#define BURGWAR_CORELIB_PLAYERMOVEMENTCONTROLLER_HPP

#include <CoreLib/Export.hpp>
#include <Nazara/Math/Vector2.hpp>
#include <NDK/Entity.hpp>

namespace Nz
{
	class Arbiter2D;
	class RigidBody2D;
}

namespace bw
{
	class PlayerMovementComponent;
	struct PlayerInputData;

	class BURGWAR_CORELIB_API PlayerMovementController
	{
		public:
			PlayerMovementController() = default;
			virtual ~PlayerMovementController();

			virtual bool PreSolveCollision(PlayerMovementComponent& playerMovement, const Ndk::EntityHandle& collisionBody, Nz::Arbiter2D& arbiter) const;

			virtual void UpdateVelocity(const PlayerInputData& inputs, PlayerMovementComponent& playerMovement, Nz::RigidBody2D& rigidBody, const Nz::Vector2f& gravity, float damping, float dt) const = 0;
	};
}

#include <CoreLib/PlayerMovementController.inl>

#endif
