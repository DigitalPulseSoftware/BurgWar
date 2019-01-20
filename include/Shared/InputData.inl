// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/InputData.hpp>

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
