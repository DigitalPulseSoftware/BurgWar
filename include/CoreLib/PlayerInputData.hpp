// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_PLAYERINPUTDATA_HPP
#define BURGWAR_CORELIB_PLAYERINPUTDATA_HPP

#include <Nazara/Math/Vector2.hpp>

namespace bw
{
	struct PlayerInputData
	{
		Nz::Vector2f aimDirection = Nz::Vector2f::UnitX();
		bool isAttacking = false;
		bool isCrouching = false;
		bool isLookingRight = true;
		bool isJumping = false;
		bool isMovingLeft = false;
		bool isMovingRight = false;

		inline bool operator==(const PlayerInputData& rhs);
		inline bool operator!=(const PlayerInputData& rhs);
	};
}

#include <CoreLib/PlayerInputData.inl>

#endif
