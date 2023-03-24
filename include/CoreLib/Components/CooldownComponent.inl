// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Components/CooldownComponent.hpp>

namespace bw
{
	inline CooldownComponent::CooldownComponent(Nz::Time cooldown) :
	m_cooldown(cooldown),
	m_nextTriggerTime(Nz::Time::Zero())
	{
	}

	inline void CooldownComponent::SetNextTriggerTime(Nz::Time time)
	{
		m_nextTriggerTime = time;
	}

	inline bool CooldownComponent::Trigger(Nz::Time currentTime)
	{
		if (currentTime < m_nextTriggerTime)
			return false;

		m_nextTriggerTime = currentTime + m_cooldown;
		return true;
	}
}
