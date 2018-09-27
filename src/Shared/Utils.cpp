// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/Utils.hpp>

namespace bw
{
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
