// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/ElementEvents.hpp>
#include <cassert>

namespace bw
{
	constexpr bool HasReturnValue(ElementEvent event)
	{
		switch (event)
		{
#define BURGWAR_EVENT(X) case ElementEvent::X: return !std::is_same_v<ElementEventData< ElementEvent::X >::ResultType, void>;

#include <CoreLib/Scripting/ElementEventList.hpp>
		}

		assert(!"Unhandled gamemode event");
		return false;
	}
}
