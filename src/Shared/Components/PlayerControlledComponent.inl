// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/Components/PlayerControlledComponent.hpp>

namespace bw
{
	inline PlayerControlledComponent::PlayerControlledComponent() :
	m_isOnGround(false)
	{
	}

	inline bool PlayerControlledComponent::IsOnGround() const
	{
		return m_isOnGround;
	}

	inline bool PlayerControlledComponent::UpdateGroundState(bool isOnGround)
	{
		if (m_isOnGround == isOnGround)
			return false;

		m_isOnGround = isOnGround;
		return true;
	}
}
