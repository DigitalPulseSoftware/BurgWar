// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Components/CollisionDataComponent.hpp>

namespace bw
{
	Nz::Collider2DRef CollisionDataComponent::BuildCollider(float scale) const
	{
		if (m_colliders.empty())
		{
			// No collider
			return nullptr;
		}
		else if (m_colliders.size() == 1)
		{
			// Single collider
			return ToCollider(m_colliders.front(), scale, m_hasCollisionCallbacks);
		}
		else
		{
			// Multiple colliders

			std::vector<Nz::Collider2DRef> simpleColliders;
			simpleColliders.reserve(m_colliders.size());

			for (const auto& collider : m_colliders)
				simpleColliders.emplace_back(ToCollider(collider, scale, m_hasCollisionCallbacks));

			Nz::CompoundCollider2DRef compound = Nz::CompoundCollider2D::New(std::move(simpleColliders));
			compound->OverridesCollisionProperties(false);

			return compound;
		}
	}

	Nz::Collider2DRef CollisionDataComponent::ToCollider(const Collider& collider, float scale, bool hasCollisionCallbacks)
	{
		return std::visit([&](auto&& arg) -> Nz::Collider2DRef
		{
			using T = std::decay_t<decltype(arg)>;

			Nz::Collider2DRef collider;

			if constexpr (std::is_same_v<T, CircleCollider>)
				collider = Nz::CircleCollider2D::New(arg.radius * scale, arg.offset * scale);
			else if constexpr (std::is_same_v<T, RectangleCollider>)
			{
				Nz::Rectf scaledRect = arg.data;
				scaledRect.x *= scale;
				scaledRect.y *= scale;
				scaledRect.width *= scale;
				scaledRect.height *= scale;

				collider = Nz::BoxCollider2D::New(scaledRect);
			}
			else if constexpr (std::is_same_v<T, SegmentCollider>)
				collider = Nz::SegmentCollider2D::New(arg.from * scale, arg.fromNeighbor * scale, arg.to * scale, arg.toNeighbor * scale);
			else
				static_assert(AlwaysFalse<T>::value, "non-exhaustive visitor");

			collider->SetElasticity(arg.physics.elasticity);
			collider->SetFriction(arg.physics.friction);
			collider->SetSurfaceVelocity(arg.physics.surfaceVelocity);
			collider->SetTrigger(arg.physics.isTrigger);

			collider->SetCollisionId((hasCollisionCallbacks) ? 1 : 0);

			return collider;

		}, collider);
	}

	Ndk::ComponentIndex CollisionDataComponent::componentIndex;
}

