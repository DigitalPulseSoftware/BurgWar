// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Components/CollisionDataComponent.hpp>
#include <CoreLib/Utils.hpp>

namespace bw
{
	std::shared_ptr<Nz::ChipmunkCollider2D> CollisionDataComponent::BuildCollider(float scale) const
	{
		if (m_colliders.empty())
		{
			// No collider
			return nullptr;
		}
		else if (m_colliders.size() == 1)
		{
			// Single collider
			return ToCollider(m_colliders.front(), scale);
		}
		else
		{
			// Multiple colliders

			std::vector<std::shared_ptr<Nz::ChipmunkCollider2D>> simpleColliders;
			simpleColliders.reserve(m_colliders.size());

			for (const auto& collider : m_colliders)
				simpleColliders.emplace_back(ToCollider(collider, scale));

			std::shared_ptr<Nz::ChipmunkCompoundCollider2D> compound = std::make_shared<Nz::ChipmunkCompoundCollider2D>(std::move(simpleColliders));
			compound->OverridesCollisionProperties(false);

			return compound;
		}
	}

	std::shared_ptr<Nz::ChipmunkCollider2D> CollisionDataComponent::ToCollider(const Collider& collider, float scale)
	{
		return std::visit([&](auto&& arg) -> std::shared_ptr<Nz::ChipmunkCollider2D>
		{
			using T = std::decay_t<decltype(arg)>;

			std::shared_ptr<Nz::ChipmunkCollider2D> collider;

			if constexpr (std::is_same_v<T, CircleCollider>)
				collider = std::make_shared<Nz::ChipmunkCircleCollider2D>(arg.radius * scale, arg.offset * scale);
			else if constexpr (std::is_same_v<T, RectangleCollider>)
			{
				Nz::Rectf scaledRect = arg.data;
				scaledRect.x *= scale;
				scaledRect.y *= scale;
				scaledRect.width *= scale;
				scaledRect.height *= scale;

				collider = std::make_shared<Nz::ChipmunkBoxCollider2D>(scaledRect);
			}
			else if constexpr (std::is_same_v<T, SegmentCollider>)
				collider = std::make_shared<Nz::ChipmunkSegmentCollider2D>(arg.from * scale, arg.fromNeighbor * scale, arg.to * scale, arg.toNeighbor * scale);
			else
				static_assert(AlwaysFalse<T>::value, "non-exhaustive visitor");

			collider->SetCollisionId(arg.physics.colliderId);
			collider->SetElasticity(arg.physics.elasticity);
			collider->SetFriction(arg.physics.friction);
			collider->SetSurfaceVelocity(arg.physics.surfaceVelocity);
			collider->SetTrigger(arg.physics.isTrigger);

			return collider;

		}, collider);
	}
}

