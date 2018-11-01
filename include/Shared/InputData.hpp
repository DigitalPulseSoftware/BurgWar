// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_SHARED_INPUTDATA_HPP
#define BURGWAR_SHARED_INPUTDATA_HPP

namespace bw
{
	struct InputData
	{
		bool isJumping = false;
		bool isMovingLeft = false;
		bool isMovingRight = false;

		inline bool operator==(const InputData& rhs);
		inline bool operator!=(const InputData& rhs);
	};
}

#include <Shared/InputData.inl>

#endif
