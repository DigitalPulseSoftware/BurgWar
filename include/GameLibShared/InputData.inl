// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <GameLibShared/InputData.hpp>

namespace bw
{
	inline bool InputData::operator==(const InputData& rhs)
	{
		return aimDirection == rhs.aimDirection && isAttacking == rhs.isAttacking && isJumping == rhs.isJumping && isMovingLeft == rhs.isMovingLeft && isMovingRight == rhs.isMovingRight;
	}

	inline bool InputData::operator!=(const InputData& rhs)
	{
		return !operator==(rhs);
	}
}
