// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/SharedScriptingLibrary.hpp>
#include <CoreLib/Scripting/ScriptingContext.hpp>
#include <CoreLib/SharedMatch.hpp>
#include <iostream>

namespace bw
{
	SharedScriptingLibrary::SharedScriptingLibrary(SharedMatch& sharedMatch) :
	m_match(sharedMatch)
	{
	}

	SharedScriptingLibrary::~SharedScriptingLibrary() = default;

	void SharedScriptingLibrary::RegisterLibrary(ScriptingContext& context)
	{
		sol::state& luaState = context.GetLuaState();
		luaState.open_libraries();

		RegisterGlobalLibrary(context);
		RegisterMetatableLibrary(context);
		RegisterTimerLibrary(context);
	}

	void SharedScriptingLibrary::RegisterTimerLibrary(ScriptingContext& context)
	{
		sol::state& luaState = context.GetLuaState();
		luaState["engine_SetTimer"] = [&](Nz::UInt64 time, sol::object callbackObject)
		{
			m_match.GetTimerManager().PushCallback(time, [&luaState, callbackObject]()
			{
				sol::protected_function callback(luaState, sol::ref_index(callbackObject.registry_index()));

				auto result = callback();
				if (!result.valid())
				{
					sol::error err = result;
					std::cerr << "engine_SetTimer callback failed: " << err.what() << std::endl;
				}
			});
		};
	}
}
