// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_COLLIDERS_HPP
#define BURGWAR_CORELIB_COLLIDERS_HPP

#include <Nazara/Math/Rect.hpp>
#include <Nazara/Math/Vector2.hpp>
#include <variant>

namespace bw
{
	struct ColliderPhysics
	{
		float elasticity = 0.f;
		float friction = 1.f;
		Nz::Vector2f surfaceVelocity = Nz::Vector2f::Zero();
		bool isTrigger = false;
		unsigned int colliderId = 0;
	};

	struct CircleCollider
	{
		ColliderPhysics physics;
		Nz::Vector2f offset;
		float radius;
	};

	struct RectangleCollider
	{
		ColliderPhysics physics;
		Nz::Rectf data;
	};

	struct SegmentCollider
	{
		ColliderPhysics physics;
		Nz::Vector2f from;
		Nz::Vector2f fromNeighbor;
		Nz::Vector2f to;
		Nz::Vector2f toNeighbor;
	};

	using Collider = std::variant<CircleCollider, RectangleCollider, SegmentCollider>;
}

#endif
