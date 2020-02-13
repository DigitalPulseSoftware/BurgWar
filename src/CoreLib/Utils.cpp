// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Utils.hpp>
#include <array>
#include <cassert>

namespace bw
{
	std::string ByteToString(Nz::UInt64 bytes, bool speed)
	{
		constexpr std::array<const char*, 9> suffixes = { "B", "KiB", "MiB", "GiB", "TiB", "PiB", "EiB", "ZiB", "YiB" };

		std::size_t suffixIndex = 0;
		Nz::UInt64 rem = 0;
		while (bytes > 1024 && suffixIndex < suffixes.size() - 1)
		{
			rem = bytes % 1024;
			bytes /= 1024;
			suffixIndex++;
		}

		assert(suffixIndex < suffixes.size());
		std::string str = std::to_string(bytes);
		if (rem > 0)
		{
			str += ".";

			std::string decimalStr = std::to_string(1000 * rem / 1024);
			if (decimalStr.size() < 3)
				str += std::string(3 - decimalStr.size(), '0');

			str += decimalStr;
		}

		str += " ";
		str += suffixes[suffixIndex];

		if (speed)
			str += "/s";

		return str;
	}

	Nz::Vector3f DampenedString(const Nz::Vector3f& currentPos, const Nz::Vector3f& targetPos, float frametime, float springStrength)
	{
		// Lynix: I didn't write this function, I took it from http://nccastaff.bournemouth.ac.uk/jmacey/RobTheBloke/www/opengl_programming.html#4 (Floaty Camera Example)

		// calculate the displacement between the target and the current position
		Nz::Vector3f displacement = targetPos - currentPos;

		// whats the distance between them?
		float displacementLength = displacement.GetLength();

		// Stops small position fluctuations (integration errors probably - since only using euler)
		if (Nz::NumberEquals(displacementLength, 0.f))
			return currentPos;

		float invDisplacementLength = 1.f / displacementLength;

		const float dampConstant = 0.000065f; // Something v.small to offset 1/ displacement length

											  // the strength of the spring increases the further away the camera is from the target.
		float springMagitude = springStrength * displacementLength + dampConstant * invDisplacementLength;

		// Normalise the displacement and scale by the spring magnitude
		// and the amount of time passed
		float scalar = std::min(invDisplacementLength * springMagitude * frametime, 1.f);
		displacement *= scalar;

		// move the camera a bit towards the target
		return currentPos + displacement;
	}
}
