// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_SHARED_INPUTDATA_HPP
#define BURGWAR_SHARED_INPUTDATA_HPP

#include <Nazara/Math/Vector2.hpp>

namespace bw
{
	struct InputData
	{
		Nz::Vector2f aimDirection = Nz::Vector2f::UnitX();
		bool isAttacking = false;
		bool isJumping = false;
		bool isMovingLeft = false;
		bool isMovingRight = false;

		inline bool operator==(const InputData& rhs);
		inline bool operator!=(const InputData& rhs);
	};
}

#include <GameLibShared/InputData.inl>

#endif
