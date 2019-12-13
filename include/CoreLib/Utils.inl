// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Utils.hpp>
#include <cassert>

namespace bw
{
	inline Nz::RadianAnglef AngleFromQuaternion(const Nz::Quaternionf& quat)
	{
		float siny_cosp = 2.f * (quat.w * quat.z + quat.x * quat.y);
		float cosy_cosp = 1.f - 2.f * (quat.y * quat.y + quat.z * quat.z);

		return std::atan2(siny_cosp, cosy_cosp); //<FIXME: not very efficient
	}

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
