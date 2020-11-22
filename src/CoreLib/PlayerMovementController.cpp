// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/PlayerMovementController.hpp>

namespace bw
{
	PlayerMovementController::~PlayerMovementController() = default;
	
	bool PlayerMovementController::PreSolveCollision(PlayerMovementComponent& /*playerMovement*/, const Ndk::EntityHandle& /*collisionBody*/, Nz::Arbiter2D& /*arbiter*/) const
	{
		return true;
	}
}
