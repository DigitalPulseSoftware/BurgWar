// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/ServerAppConfig.hpp>
#include <Server/ServerAppComponent.hpp>

namespace bw
{
	ServerAppConfig::ServerAppConfig(ServerAppComponent& app) :
	SharedAppConfig(app)
	{
		RegisterStringOption("ServerSettings.Gamemode");
		RegisterStringOption("ServerSettings.MapPath");
		RegisterIntegerOption("ServerSettings.MaxPlayerCount", 1, 0xFFFF, 16);
		RegisterIntegerOption("ServerSettings.Port", 1, 0xFFFF, 14768);
		RegisterBoolOption("ServerSettings.SleepWhenEmpty", true);

		RegisterStringOption("ServerSettings.Description", "", [](std::string value) -> Nz::Result<std::string, std::string>
		{
			if (value.size() > 1024)
				return Nz::Err("description is too long");

			return Nz::Ok(std::move(value));
		});

		RegisterStringOption("ServerSettings.Name", "a server has no name", [](std::string value) -> Nz::Result<std::string, std::string>
		{
			if (value.empty())
				return Nz::Err("name cannot be empty");

			if (value.size() > 32)
				return Nz::Err("name is too long");

			return Nz::Ok(std::move(value));
		});
	}
}
