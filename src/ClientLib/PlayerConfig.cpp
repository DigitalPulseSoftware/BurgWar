// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/PlayerConfig.hpp>

namespace bw
{
	PlayerConfig::PlayerConfig(BurgAppComponent& app) :
	ConfigFile(app)
	{
		RegisterStringOption("JoinServer.Address", "localhost");
		RegisterStringOption("StartServer.Description", "");
		RegisterBoolOption("StartServer.ListServer", false);
		RegisterStringOption("StartServer.Gamemode", "deathmatch");
		RegisterStringOption("StartServer.Map", "beta_map");
		RegisterStringOption("StartServer.Name", "A server has no name");
		RegisterIntegerOption("JoinServer.Port", 0, 0xFFFF, 14768);
		RegisterIntegerOption("StartServer.Port", 0, 0xFFFF, 14768);
		RegisterIntegerOption("Sound.GlobalVolume", 0, 100, 80);
		RegisterIntegerOption("Sound.EffectVolume", 0, 100, 100);
		RegisterIntegerOption("Sound.MusicVolume", 0, 100, 60);
		
		RegisterStringOption("Player.Name", "mingebag", [](std::string value) -> Nz::Result<std::string, std::string>
		{
			if (value.empty())
				return Nz::Err("name cannot be empty");

			if (value.size() > 32)
				return Nz::Err("name is too long");

			return Nz::Ok(std::move(value));
		});
	}
}
