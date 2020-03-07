// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/ServerConfigFile.hpp>
#include <Server/ServerApp.hpp>

namespace bw
{
	ServerConfigFile::ServerConfigFile(ServerApp& app) :
	SharedConfigFile(app)
	{
		RegisterStringOption("GameSettings.MapFile");
	}
}
