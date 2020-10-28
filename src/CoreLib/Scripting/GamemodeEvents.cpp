// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/GamemodeEvents.hpp>
#include <CoreLib/Utils.hpp>
#include <cassert>
#include <cctype>

namespace bw
{
	std::optional<GamemodeEvent> RetrieveGamemodeEvent(const std::string_view& eventName)
	{
		for (std::size_t i = 0; i < GamemodeEventCount; ++i)
		{
			GamemodeEvent event = static_cast<GamemodeEvent>(i);
			if (StringEqual(ToString(event), eventName))
				return event;
		}

		return {};
	}

	std::string_view ToString(GamemodeEvent event)
	{
		switch (event)
		{
#define BURGWAR_EVENT(X) case GamemodeEvent::X: return #X;

#include <CoreLib/Scripting/GamemodeEventList.hpp>
		}

		assert(!"Unhandled gamemode event");
		return nullptr;
	}
}
