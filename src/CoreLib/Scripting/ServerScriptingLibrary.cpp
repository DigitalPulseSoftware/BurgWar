// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/ServerScriptingLibrary.hpp>
#include <CoreLib/Match.hpp>
#include <CoreLib/Player.hpp>
#include <iostream>

namespace bw
{
	ServerScriptingLibrary::ServerScriptingLibrary(Match& match) :
	SharedScriptingLibrary(match)
	{
	}

	void ServerScriptingLibrary::RegisterLibrary(SharedScriptingContext& context)
	{
		SharedScriptingLibrary::RegisterLibrary(context);

		sol::state& state = context.GetLuaState();
		state["CLIENT"] = false;
		state["SERVER"] = true;

		state.set_function("RegisterClientScript", [&](const std::string& path)
		{
			GetMatch().RegisterClientScript(context.GetCurrentFolder() / path);
		});

		context.Load("../../scripts/autorun");
		RegisterPlayer(context);

	}

	void ServerScriptingLibrary::RegisterPlayer(SharedScriptingContext& context)
	{
		sol::state& state = context.GetLuaState();
		state.new_usertype<PlayerHandle>("Player", 
			"new", sol::no_constructor,
			"GetName", [](const PlayerHandle& player) -> std::string
			{
				if (!player)
					return "<Disconnected>";

				return player->GetName();
			},
			"Spawn", [](const PlayerHandle& player)
			{
				if (!player)
					return;

				return player->Spawn();
			}
		);
	}

	Match& ServerScriptingLibrary::GetMatch()
	{
		return static_cast<Match&>(GetSharedMatch());
	}
}
