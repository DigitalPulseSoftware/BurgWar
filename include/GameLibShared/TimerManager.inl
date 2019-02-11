// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <GameLibShared/TimerManager.hpp>

namespace bw
{
	inline TimerManager::TimerManager(BurgApp& app) :
	m_app(app)
	{
	}

	inline void TimerManager::PushCallback(Nz::UInt64 expirationTime, Callback callback)
	{
		Timer& timer = m_pendingTimers.emplace_back();
		timer.callback = std::move(callback);
		timer.expirationTime = m_app.GetAppTime() + expirationTime;
	}

	inline void TimerManager::Update()
	{
		Nz::UInt64 now = m_app.GetAppTime();

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
