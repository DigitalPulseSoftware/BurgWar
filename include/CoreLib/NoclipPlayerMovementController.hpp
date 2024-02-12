// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_NOCLIPPLAYERMOVEMENTCONTROLLER_HPP
#define BURGWAR_CORELIB_NOCLIPPLAYERMOVEMENTCONTROLLER_HPP

#include <CoreLib/Export.hpp>
#include <CoreLib/PlayerMovementController.hpp>

namespace bw
{
	class BURGWAR_CORELIB_API NoclipPlayerMovementController : public PlayerMovementController
	{
		public:
			NoclipPlayerMovementController() = default;
			~NoclipPlayerMovementController() = default;

			void UpdateVelocity(const PlayerInputData& inputs, PlayerMovementComponent& playerMovement, Nz::RigidBody2D& rigidBody, const Nz::Vector2f& gravity, float damping, float dt) const override;
	};
}

#include <CoreLib/NoclipPlayerMovementController.inl>

#endif
