// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Scripting/ClientScriptingLibrary.hpp>
#include <ClientLib/LocalMatch.hpp>
#include <iostream>

namespace bw
{
	ClientScriptingLibrary::ClientScriptingLibrary(LocalMatch& match) :
	SharedScriptingLibrary(match)
	{
	}

	void ClientScriptingLibrary::RegisterLibrary(ScriptingContext& context)
	{
		SharedScriptingLibrary::RegisterLibrary(context);

		sol::state& state = context.GetLuaState();
		state["CLIENT"] = true;
		state["SERVER"] = false;

		state["RegisterClientScript"] = []() {}; // Dummy function

		RegisterScriptLibrary(context);

		context.Load("autorun");
	}

	void ClientScriptingLibrary::RegisterScriptLibrary(ScriptingContext& context)
	{
		sol::state& state = context.GetLuaState();
		sol::table script = state.create_table();

		script["ReloadAll"] = [this]()
		{
			LocalMatch& match = GetMatch();
			match.LoadScripts();
		};

		state["scripts"] = script;
	}

	LocalMatch& ClientScriptingLibrary::GetMatch()
	{
		return static_cast<LocalMatch&>(GetSharedMatch());
	}
}
