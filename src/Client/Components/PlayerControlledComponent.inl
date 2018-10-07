// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/Components/PlayerControlledComponent.hpp>

namespace bw
{
	inline PlayerControlledComponent::PlayerControlledComponent() :
	m_isFacingRight(true),
	m_isOnGround(false)
	{
	}

	inline bool PlayerControlledComponent::IsFacingRight() const
	{
		return m_isFacingRight;
	}

	inline bool PlayerControlledComponent::IsOnGround() const
	{
		return m_isOnGround;
	}

	inline bool PlayerControlledComponent::UpdateFacingRightState(bool isFacingRight)
	{
		if (m_isFacingRight == isFacingRight)
			return false;

		m_isFacingRight = isFacingRight;
		return true;
	}

	inline bool PlayerControlledComponent::UpdateGroundState(bool isOnGround)
	{
		if (m_isOnGround == isOnGround)
			return false;

		m_isOnGround = isOnGround;
		return true;
	}
}
