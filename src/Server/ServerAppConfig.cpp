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
		RegisterBoolOption("ServerSettings.DisableWhenEmpty", true);
		RegisterStringOption("ServerSettings.Gamemode");
		RegisterStringOption("ServerSettings.MapPath");
		RegisterIntegerOption("ServerSettings.MaxPlayerCount", 1, 0xFFFF, 16);
		RegisterIntegerOption("ServerSettings.Port", 1, 0xFFFF, 14768);

		RegisterStringOption("ServerSettings.Description", "", [](std::string value) -> tl::expected<std::string, std::string>
		{
			if (value.size() > 1024)
				return tl::make_unexpected("description is too long");

			return value;
		});

		RegisterStringOption("ServerSettings.Name", "a server has no name", [](std::string value) -> tl::expected<std::string, std::string>
		{
			if (value.empty())
				return tl::make_unexpected("name cannot be empty");

			if (value.size() > 32)
				return tl::make_unexpected("name is too long");

			return value;
		});
	}
}
