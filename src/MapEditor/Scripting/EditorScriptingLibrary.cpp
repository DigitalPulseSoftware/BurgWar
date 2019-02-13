// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Scripting/EditorScriptingLibrary.hpp>
#include <CoreLib/Scripting/SharedScriptingContext.hpp>
#include <iostream>

namespace bw
{
	void EditorScriptingLibrary::RegisterLibrary(SharedScriptingContext& context)
	{
		sol::state& state = context.GetLuaState();
		state["CLIENT"] = false;
		state["SERVER"] = false;
		state["EDITOR"] = false;

		state.open_libraries();

		state["RegisterClientScript"] = []() {}; // Dummy function

		AbstractScriptingLibrary::RegisterGlobalLibrary(context);
		AbstractScriptingLibrary::RegisterMetatableLibrary(context);

		context.Load("autorun");
	}
}
