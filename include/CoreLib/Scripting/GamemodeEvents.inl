// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/GamemodeEvents.hpp>
#include <cassert>

namespace bw
{
	constexpr bool HasReturnValue(GamemodeEvent event)
	{
		switch (event)
		{
#define BURGWAR_EVENT(X) case GamemodeEvent::X: return !std::is_same_v<GamemodeEventData< GamemodeEvent::X >::ResultType, void>;

#include <CoreLib/Scripting/GamemodeEventList.hpp>
		}

		return false;
	}
}
