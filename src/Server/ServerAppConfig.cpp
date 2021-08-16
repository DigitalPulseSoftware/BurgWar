// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/ServerAppConfig.hpp>
#include <Server/ServerApp.hpp>

namespace bw
{
	ServerAppConfig::ServerAppConfig(ServerApp& app) :
	SharedAppConfig(app)
	{
		RegisterStringOption("ServerSettings.Gamemode");
		RegisterStringOption("ServerSettings.MapPath");
	}
}
