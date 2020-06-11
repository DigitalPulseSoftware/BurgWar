// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Components/CooldownComponent.hpp>

namespace bw
{
	inline CooldownComponent::CooldownComponent(Nz::UInt32 cooldown) :
	m_cooldown(cooldown),
	m_nextTriggerTime(0)
	{
	}

	inline void CooldownComponent::SetNextTriggerTime(Nz::UInt64 time)
	{
		m_nextTriggerTime = time;
	}

	inline bool CooldownComponent::Trigger(Nz::UInt64 currentTime)
	{
		if (currentTime < m_nextTriggerTime)
			return false;

		m_nextTriggerTime = currentTime + m_cooldown;
		return true;
	}
}
