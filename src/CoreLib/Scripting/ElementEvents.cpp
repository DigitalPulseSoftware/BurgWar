// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/ElementEvents.hpp>
#include <CoreLib/Utils.hpp>
#include <cassert>
#include <cctype>

namespace bw
{
	std::optional<ElementEvent> RetrieveElementEvent(const std::string_view& eventName)
	{
		for (std::size_t i = 0; i < ElementEventCount; ++i)
		{
			ElementEvent event = static_cast<ElementEvent>(i);
			if (StringEqual(ToString(event), eventName))
				return event;
		}

		return {};
	}

	std::string_view ToString(ElementEvent event)
	{
		switch (event)
		{
#define BURGWAR_EVENT(X) case ElementEvent::X: return #X;

#include <CoreLib/Scripting/ElementEventList.hpp>
		}

		assert(!"Unhandled gamemode event");
		return nullptr;
	}
}
