// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/TimerManager.hpp>

namespace bw
{
	inline TimerManager::TimerManager(SharedMatch& match) :
	m_match(match)
	{
	}

	inline void TimerManager::Clear()
	{
		m_pendingTimers.clear();
	}

	void TimerManager::PushCallback(Nz::UInt64 expirationTime, Callback callback)
	{
		Timer& timer = m_pendingTimers.emplace_back();
		timer.callback = std::move(callback);
		timer.expirationTime = expirationTime;

		//TODO: Insertion sort
	}

	void TimerManager::Update(Nz::UInt64 now)
	{
		// Use index instead of iterator because callback may push new timers
		for (std::size_t i = 0; i < m_pendingTimers.size();)
		{
			Timer& timer = m_pendingTimers[i];
			if (now > timer.expirationTime)
			{
				timer.callback();

				m_pendingTimers.erase(m_pendingTimers.begin() + i);
			}
			else
				++i;
		}
	}
}
