// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/ScriptedEvent.hpp>

namespace bw
{
	ScriptedEvent InitEventFromLua(std::size_t index, const sol::table& table)
	{
		ScriptedEvent event;
		event.index = index;
		event.name = table["Name"];

		sol::object eventCombinator = table["Combinator"];
		if (eventCombinator)
			event.combinator = eventCombinator.as<sol::main_protected_function>();

		event.returnType = table.get_or("ReturnType", std::string());

		return event;
	}
}
