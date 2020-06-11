// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Utils.hpp>
#include <cassert>
#include <cmath>

namespace bw
{
	inline Nz::RadianAnglef AngleFromQuaternion(const Nz::Quaternionf& quat)
	{
		float siny_cosp = 2.f * (quat.w * quat.z + quat.x * quat.y);
		float cosy_cosp = 1.f - 2.f * (quat.y * quat.y + quat.z * quat.z);

		return std::atan2(siny_cosp, cosy_cosp); //<FIXME: not very efficient
	}

	template<typename T> 
	Nz::Vector2<T> AlignPosition(Nz::Vector2<T> position, const Nz::Vector2<T>& alignment)
	{
		assert(alignment.x > T(0));
		assert(alignment.y > T(0));

		position.x = std::round(position.x / alignment.x) * alignment.x;
		position.y = std::round(position.y / alignment.y) * alignment.y;

		return position;
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

	std::string ReplaceStr(std::string str, const std::string_view& from, const std::string_view& to)
	{
		if (str.empty())
			return str;

		std::size_t startPos = 0;
		while ((startPos = str.find(from, startPos)) != std::string::npos)
		{
			str.replace(startPos, from.length(), to);
			startPos += to.length();
		}

		return str;
	}

	template<typename F> 
	bool SplitString(const std::string_view& str, const std::string_view& token, F&& func)
	{
		std::size_t pos = 0;
		std::size_t previousPos = 0;
		while ((pos = str.find(token, previousPos)) != std::string::npos)
		{
			std::size_t splitPos = previousPos;
			previousPos = pos + token.size();

			if (!func(str.substr(splitPos, pos - splitPos)))
				return false;
		}

		return func(str.substr(previousPos));
	}

	template<typename F>
	bool SplitStringAny(const std::string_view& str, const std::string_view& token, F&& func)
	{
		std::size_t pos = 0;
		std::size_t previousPos = 0;
		while ((pos = str.find_first_of(token, previousPos)) != std::string::npos)
		{
			std::size_t splitPos = previousPos;
			previousPos = pos + 1;

			if (!func(str.substr(splitPos, pos - splitPos)))
				return false;
		}

		return func(str.substr(previousPos));
	}

	template<typename E> auto UnderlyingCast(E value) -> std::underlying_type_t<E>
	{
		return static_cast<std::underlying_type_t<E>>(value);
	}
}
