// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/ClientConfigFile.hpp>
#include <Client/ClientApp.hpp>

namespace bw
{
	ClientConfigFile::ClientConfigFile(ClientApp& app) :
	SharedConfigFile(app)
	{
		RegisterStringOption("Debug.ShowConnectionData");
		RegisterBoolOption("Debug.ShowServerGhosts");
		RegisterStringOption("GameSettings.MapFile");
		RegisterIntegerOption("WindowSettings.AntialiasingLevel", 0, 16);
		RegisterBoolOption("WindowSettings.Fullscreen");
		RegisterBoolOption("WindowSettings.VSync");
		RegisterIntegerOption("WindowSettings.FPSLimit", 0, 1000);
		RegisterIntegerOption("WindowSettings.Height", 0, 0xFFFFFFFF);
		RegisterIntegerOption("WindowSettings.Width", 0, 0xFFFFFFFF);
	}
}
