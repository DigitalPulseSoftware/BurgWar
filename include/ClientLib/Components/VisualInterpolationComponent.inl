// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Components/VisualInterpolationComponent.hpp>

namespace bw
{
	inline VisualInterpolationComponent::VisualInterpolationComponent() :
	m_lastRotation(Nz::RadianAnglef::Zero()),
	m_lastPosition(Nz::Vector2f::Zero())
	{
	}

	inline const Nz::Vector2f& VisualInterpolationComponent::GetLastPosition()
	{
		return m_lastPosition;
	}
	
	inline const Nz::RadianAnglef& VisualInterpolationComponent::GetLastRotation()
	{
		return m_lastRotation;
	}
	
	inline void VisualInterpolationComponent::UpdateLastStates(const Nz::Vector2f& position, const Nz::RadianAnglef& rotation)
	{
		m_lastPosition = position;
		m_lastRotation = rotation;
	}
}
