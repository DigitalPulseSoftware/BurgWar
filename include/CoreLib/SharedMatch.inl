// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/SharedMatch.hpp>
#include <cassert>

namespace bw
{
	inline SharedMatch::SharedMatch(BurgApp& app, float tickDuration) :
	m_timerManager(app),
	m_currentTick(0),
	m_currentTime(0),
	m_floatingTime(0.f),
	m_tickDuration(tickDuration),
	m_tickTimer(0.f)
	{
	}

	inline Nz::UInt16 SharedMatch::GetCurrentTick() const
	{
		return m_currentTick;
	}

	inline Nz::UInt64 SharedMatch::GetCurrentTime() const
	{
		return m_currentTime;
	}

	inline float SharedMatch::GetTickDuration() const
	{
		return m_tickDuration;
	}

	inline TimerManager& SharedMatch::GetTimerManager()
	{
		return m_timerManager;
	}
}
