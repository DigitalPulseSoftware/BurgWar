// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Utils.hpp>
#include <cassert>

namespace bw
{
	template<typename T>
	bool IsMoreRecent(T a, T b)
	{
		static constexpr T half = std::numeric_limits<T>::max() / 2;

		if (a > b)
		{
			if (a - b <= half)
				return true;
		}
		else if (b > a)
		{
			if (b - a > half)
				return true;
		}

		return false;
	}
}
