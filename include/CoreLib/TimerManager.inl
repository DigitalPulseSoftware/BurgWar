// Copyright (C) 2019 Jérôme Leclercq
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
		for (auto it = m_pendingTimers.begin(); it != m_pendingTimers.end();)
		{
			Timer& timer = *it;
			if (now > timer.expirationTime)
			{
				timer.callback();

				it = m_pendingTimers.erase(it);
			}
			else
				++it;
		}
	}
}
