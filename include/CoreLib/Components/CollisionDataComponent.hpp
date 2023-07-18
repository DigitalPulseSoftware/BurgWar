// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_COMPONENTS_COLLISIONDATA_HPP
#define BURGWAR_CORELIB_COMPONENTS_COLLISIONDATA_HPP

#include <CoreLib/Colliders.hpp>
#include <CoreLib/Export.hpp>
#include <Nazara/ChipmunkPhysics2D/ChipmunkCollider2D.hpp>
#include <vector>

namespace bw
{
	class BURGWAR_CORELIB_API CollisionDataComponent
	{
		friend class ColliderSystem;

		public:
			CollisionDataComponent() = default;
			CollisionDataComponent(const CollisionDataComponent&) = default;
			CollisionDataComponent(CollisionDataComponent&&) = default;
			~CollisionDataComponent() = default;

			inline void AddCollider(Collider collider);

			std::shared_ptr<Nz::ChipmunkCollider2D> BuildCollider(float scale = 1.f) const;

			inline const std::vector<Collider>& GetColliders() const;

			CollisionDataComponent& operator=(const CollisionDataComponent&) = default;
			CollisionDataComponent& operator=(CollisionDataComponent&&) = default;

		private:
			static std::shared_ptr<Nz::ChipmunkCollider2D> ToCollider(const Collider& collider, float scale);

			std::vector<Collider> m_colliders;
	};
}

#include <CoreLib/Components/CollisionDataComponent.inl>

#endif
