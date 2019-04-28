// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/SharedMatch.hpp>
#include <cassert>

namespace bw
{
	SharedMatch::~SharedMatch() = default;

	void SharedMatch::Update(float elapsedTime)
	{
		m_timerManager.Update();

		m_tickTimer += std::min(elapsedTime, 0.25f);
		while (m_tickTimer >= m_tickDuration)
		{
			m_tickTimer -= m_tickDuration;

			OnTick();

			m_currentTick++;
			m_floatingTime += m_tickDuration * 1000.f;
			Nz::UInt64 elapsedTimeMs = static_cast<Nz::UInt64>(m_floatingTime);
			m_currentTime += elapsedTimeMs;
			m_floatingTime -= elapsedTimeMs;
		}
	}
}
