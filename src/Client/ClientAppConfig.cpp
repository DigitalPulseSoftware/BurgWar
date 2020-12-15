// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/ClientAppConfig.hpp>
#include <Client/ClientApp.hpp>

namespace bw
{
	ClientAppConfig::ClientAppConfig(ClientApp& app) :
	SharedAppConfig(app)
	{
		RegisterStringOption("Debug.ShowConnectionData");
		RegisterBoolOption("Debug.ShowServerGhosts");
		RegisterBoolOption("Debug.ShowVersion", true);
		RegisterIntegerOption("WindowSettings.AntialiasingLevel", 0, 16);
		RegisterBoolOption("WindowSettings.Fullscreen");
		RegisterBoolOption("WindowSettings.VSync");
		RegisterIntegerOption("WindowSettings.FPSLimit", 0, 1000);
		RegisterIntegerOption("WindowSettings.Height", 0, 0xFFFFFFFF);
		RegisterIntegerOption("WindowSettings.Width", 0, 0xFFFFFFFF);
	}
}
