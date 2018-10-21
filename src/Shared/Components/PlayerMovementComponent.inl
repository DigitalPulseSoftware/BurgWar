// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/Components/PlayerControlledComponent.hpp>

namespace bw
{
	inline PlayerMovementComponent::PlayerMovementComponent() :
	m_isAirControlling(false),
	m_isFacingRight(true)
	{
	}

	inline bool PlayerMovementComponent::IsAirControlling() const
	{
		return m_isAirControlling;
	}

	inline bool PlayerMovementComponent::IsFacingRight() const
	{
		return m_isFacingRight;
	}

	inline bool PlayerMovementComponent::UpdateAirControlState(bool isAirControlling)
	{
		if (m_isAirControlling == isAirControlling)
			return false;

		m_isAirControlling = isAirControlling;
		return true;
	}

	inline bool PlayerMovementComponent::UpdateFacingRightState(bool isFacingRight)
	{
		if (m_isFacingRight == isFacingRight)
			return false;

		m_isFacingRight = isFacingRight;
		return true;
	}
}
