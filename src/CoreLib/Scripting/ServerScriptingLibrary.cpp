// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/ServerScriptingLibrary.hpp>
#include <CoreLib/Match.hpp>
#include <CoreLib/Player.hpp>

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

		state.set_function("RegisterClientAssets", [&](sol::this_state L, const sol::object& paths) -> std::pair<bool, sol::object>
		{
			if (!paths.is<sol::table>() && !paths.is<std::string>())
				throw std::runtime_error("expected table or string");

			try
			{
				if (paths.is<sol::table>())
				{
					sol::table pathTable = paths.as<sol::table>();
					for (auto&& [k, v] : pathTable)
					{
						if (v.is<std::string>())
							GetMatch().RegisterAsset(v.as<std::string>());
					}
				}
				else if (paths.is<std::string>())
				{
					GetMatch().RegisterAsset(paths.as<std::string>());
				}

				return { true, sol::nil };
			}
			catch (const std::exception& err)
			{
				bwLog(GetMatch().GetLogger(), LogLevel::Warning, "RegisterClientAssets failed: {}", err.what());
				return { false, sol::make_object<std::string>(L, err.what()) };
			}
		});

		state.set_function("RegisterClientScript", [&](sol::this_state L, const std::string& path) -> std::pair<bool, sol::object>
		{
			try
			{
				GetMatch().RegisterClientScript(context.GetCurrentFolder() / path);
				return { true, sol::nil };
			}
			catch (const std::exception& err)
			{
				bwLog(GetMatch().GetLogger(), LogLevel::Warning, "RegisterClientScript failed: {}", err.what());
				return { false, sol::make_object<std::string>(L, err.what()) };

			}
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
			},
			"UpdateLayer", [](const PlayerHandle& player, sol::object layerIndex)
			{
				if (!player)
					return;
			
				if (layerIndex != sol::nil)
					player->UpdateLayer(layerIndex.as<std::size_t>());
				else
					player->UpdateLayer(Player::NoLayer);
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
