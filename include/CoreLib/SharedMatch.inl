// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/SharedMatch.hpp>
#include <cassert>

namespace bw
{
	inline Nz::UInt64 SharedMatch::GetCurrentTick() const
	{
		return m_currentTick;
	}

	inline auto SharedMatch::GetLogger() -> MatchLogger&
	{
		return m_logger;
	}

	inline const std::string& SharedMatch::GetName() const
	{
		return m_name;
	}

	inline Nz::UInt16 SharedMatch::GetNetworkTick() const
	{
		return GetNetworkTick(m_currentTick);
	}

	inline Nz::UInt16 SharedMatch::GetNetworkTick(Nz::UInt64 tick) const
	{
		return static_cast<Nz::UInt16>(tick % (0xFFFFU + 1));
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
