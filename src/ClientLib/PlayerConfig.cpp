// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/PlayerConfig.hpp>

namespace bw
{
	PlayerConfig::PlayerConfig(BurgApp& app) :
	ConfigFile(app)
	{
		RegisterStringOption("Player.Name", "mingebag");
		RegisterStringOption("JoinServer.Address", "localhost");
		RegisterStringOption("StartServer.Gamemode", "deathmatch");
		RegisterStringOption("StartServer.Map", "beta_map");
		RegisterIntegerOption("JoinServer.Port", 0, 0xFFFF, 14768);
		RegisterIntegerOption("StartServer.Port", 0, 0xFFFF, 14768);
		RegisterIntegerOption("Sound.GlobalVolume", 0, 100, 80);
		RegisterIntegerOption("Sound.EffectVolume", 0, 100, 100);
		RegisterIntegerOption("Sound.MusicVolume", 0, 100, 60);
	}
}
