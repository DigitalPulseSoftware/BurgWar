// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/SharedMatch.hpp>
#include <cassert>

namespace bw
{
	inline SharedMatch::SharedMatch(BurgApp& app) :
	m_timerManager(app)
	{
	}

	inline TimerManager& SharedMatch::GetTimerManager()
	{
		return m_timerManager;
	}
}
