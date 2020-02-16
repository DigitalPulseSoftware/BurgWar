// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/PlayerInputData.hpp>

namespace bw
{
	inline bool PlayerInputData::operator==(const PlayerInputData& rhs)
	{
		return aimDirection == rhs.aimDirection && 
		       isAttacking == rhs.isAttacking && 
		       isCrouching == rhs.isCrouching &&
		       isLookingRight == rhs.isLookingRight &&
		       isJumping == rhs.isJumping && 
		       isMovingLeft == rhs.isMovingLeft && 
		       isMovingRight == rhs.isMovingRight;
	}

	inline bool PlayerInputData::operator!=(const PlayerInputData& rhs)
	{
		return !operator==(rhs);
	}
}
