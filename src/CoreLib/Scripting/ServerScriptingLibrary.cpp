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

	void ServerScriptingLibrary::RegisterLibrary(ScriptingContext& context)
	{
		SharedScriptingLibrary::RegisterLibrary(context);

		sol::state& state = context.GetLuaState();
		state["CLIENT"] = false;
		state["SERVER"] = true;

		state.set_function("RegisterClientScript", [&](const std::string& path)
		{
			GetMatch().RegisterClientScript(context.GetCurrentFolder() / path);
		});

		RegisterPlayer(context);

		RegisterScriptLibrary(context);

		context.Load("autorun");
	}

	void ServerScriptingLibrary::RegisterPlayer(ScriptingContext& context)
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
			"GiveWeapon", [](const PlayerHandle& player, std::string weaponName)
			{
				if (!player)
					return false;

				return player->GiveWeapon(std::move(weaponName));
			},
			"HasWeapon", [](const PlayerHandle& player, const std::string& weaponName)
			{
				if (!player)
					return false;

				return player->HasWeapon(weaponName);
			},
			"RemoveWeapon", [](const PlayerHandle& player, const std::string& weaponName)
			{
				if (!player)
					return;

				return player->RemoveWeapon(weaponName);
			},
			"Spawn", [](const PlayerHandle& player)
			{
				if (!player)
					return;

				return player->Spawn();
			}
		);
	}

	void ServerScriptingLibrary::RegisterScriptLibrary(ScriptingContext& context)
	{
		sol::state& state = context.GetLuaState();
		sol::table script = state.create_table();

		script["ReloadAll"] = [this]()
		{
			Match& match = GetMatch();
			match.ReloadScripts();
		};

		state["scripts"] = script;
	}

	Match& ServerScriptingLibrary::GetMatch()
	{
		return static_cast<Match&>(GetSharedMatch());
	}
}
