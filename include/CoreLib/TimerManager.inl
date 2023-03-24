// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/TimerManager.hpp>

namespace bw
{
	inline void TimerManager::Clear()
	{
		m_pendingTimers.clear();
	}

	inline void TimerManager::PushCallback(Nz::Time expirationTime, Callback callback)
	{
		Timer& timer = m_pendingTimers.emplace_back();
		timer.callback = std::move(callback);
		timer.expirationTime = expirationTime;

		//TODO: Insertion sort
	}

	inline void TimerManager::Update(Nz::Time now)
	{
		// Use index instead of iterator because callback may push new timers
		for (std::size_t i = 0; i < m_pendingTimers.size();)
		{
			if (now > m_pendingTimers[i].expirationTime)
			{
				auto it = m_pendingTimers.begin() + i;

				Timer timer = std::move(*it);
				m_pendingTimers.erase(it);

				timer.callback();
			}
			else
				++i;
		}
	}
}
