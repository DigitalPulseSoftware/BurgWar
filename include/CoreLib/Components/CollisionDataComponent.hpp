// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_COMPONENTS_COLLISIONDATA_HPP
#define BURGWAR_CORELIB_COMPONENTS_COLLISIONDATA_HPP

#include <CoreLib/Colliders.hpp>
#include <CoreLib/Export.hpp>
#include <Nazara/Physics2D/Collider2D.hpp>
#include <NDK/Component.hpp>
#include <vector>

namespace bw
{
	class BURGWAR_CORELIB_API CollisionDataComponent : public Ndk::Component<CollisionDataComponent>
	{
		friend class ColliderSystem;

		public:
			CollisionDataComponent() = default;
			~CollisionDataComponent() = default;

			inline void AddCollider(Collider collider);

			Nz::Collider2DRef BuildCollider(float scale = 1.f) const;

			inline const std::vector<Collider>& GetColliders() const;

			static Ndk::ComponentIndex componentIndex;

		private:
			static Nz::Collider2DRef ToCollider(const Collider& collider, float scale);

			std::vector<Collider> m_colliders;
	};
}

#include <CoreLib/Components/CollisionDataComponent.inl>

#endif
