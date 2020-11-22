// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Components/CollisionDataComponent.hpp>

namespace bw
{
	inline void CollisionDataComponent::AddCollider(Collider collider)
	{
		m_colliders.emplace_back(std::move(collider));
	}

	inline const std::vector<Collider>& CollisionDataComponent::GetColliders() const
	{
		return m_colliders;
	}
}
