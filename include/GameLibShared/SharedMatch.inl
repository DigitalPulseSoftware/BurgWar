// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <GameLibShared/SharedMatch.hpp>
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
