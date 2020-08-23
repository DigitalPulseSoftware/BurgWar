// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Components/CollisionDataComponent.hpp>

namespace bw
{
	inline CollisionDataComponent::CollisionDataComponent() :
	m_hasCollisionCallbacks(false)
	{
	}

	inline void CollisionDataComponent::AddCollider(Collider collider)
	{
		m_colliders.emplace_back(std::move(collider));
	}

	inline void CollisionDataComponent::DisableCollisionCallbacks()
	{
		return EnableCollisionCallbacks(false);
	}

	inline void CollisionDataComponent::EnableCollisionCallbacks(bool enable)
	{
		m_hasCollisionCallbacks = enable;
	}

	inline const std::vector<Collider>& CollisionDataComponent::GetColliders() const
	{
		return m_colliders;
	}

	inline bool CollisionDataComponent::HasCollisionCallbacks() const
	{
		return m_hasCollisionCallbacks;
	}
}
