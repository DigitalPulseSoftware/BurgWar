// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/ServerAppComponent.hpp>
#include <Nazara/Core/AppFilesystemComponent.hpp>
#include <Nazara/Core/ApplicationBase.hpp>

namespace bw
{
	ServerAppComponent::ServerAppComponent(Nz::ApplicationBase& app) :
	BurgAppComponent(app, LogSide::Server, m_configFile),
	m_configFile(*this)
	{
		if (!m_configFile.LoadFromFile("serverconfig.lua"))
			throw std::runtime_error("failed to load config file");

		Nz::AppFilesystemComponent& appFilesystem = app.GetComponent<Nz::AppFilesystemComponent>();
		appFilesystem.Mount("assets", Nz::Utf8Path(m_configFile.GetStringValue("Resources.AssetDirectory")));
		appFilesystem.Mount("mods", Nz::Utf8Path(m_configFile.GetStringValue("Resources.ModDirectory")));
		appFilesystem.Mount("scripts", Nz::Utf8Path(m_configFile.GetStringValue("Resources.ScriptDirectory")));

		LoadMods();

		Nz::UInt16 maxPlayerCount = m_configFile.GetIntegerValue<Nz::UInt16>("ServerSettings.MaxPlayerCount");
		Nz::UInt16 serverPort = m_configFile.GetIntegerValue<Nz::UInt16>("ServerSettings.Port");
		const std::string& gamemode = m_configFile.GetStringValue("ServerSettings.Gamemode");
		const std::string& mapPath = m_configFile.GetStringValue("ServerSettings.MapPath");
		const std::string& serverDesc = m_configFile.GetStringValue("ServerSettings.Description");
		const std::string& serverName = m_configFile.GetStringValue("ServerSettings.Name");
		float tickRate = m_configFile.GetFloatValue<float>("ServerSettings.TickRate");
		bool sleepWhenEmpty = m_configFile.GetBoolValue("ServerSettings.SleepWhenEmpty");

		Match::GamemodeSettings gamemodeSettings;
		gamemodeSettings.name = gamemode;

		Match::MatchSettings matchSettings;
		matchSettings.sleepWhenEmpty = sleepWhenEmpty;
		matchSettings.description = serverDesc;
		matchSettings.maxPlayerCount = maxPlayerCount;
		matchSettings.name = serverName;
		matchSettings.port = serverPort;
		matchSettings.tickDuration = Nz::Time::Seconds(1.f / tickRate);

		// Load map
		if (!EndsWith(mapPath, ".bmap"))
		{
			if (std::filesystem::is_directory(mapPath))
				matchSettings.map = Map::LoadFromDirectory(mapPath);
			else
				matchSettings.map = Map::LoadFromBinary(mapPath + ".bmap");
		}
		else
			matchSettings.map = Map::LoadFromBinary(mapPath);

		Match::ModSettings modSettings;

		// FIXME: Allow to select enabled mods
		for (auto&& [modId, mod] : GetMods())
			modSettings.enabledMods[modId] = Match::ModSettings::ModEntry{};

		m_match = std::make_unique<Match>(*this, std::move(matchSettings), std::move(gamemodeSettings), std::move(modSettings));
	}

	void ServerAppComponent::Update(Nz::Time elapsedTime)
	{
		BurgAppComponent::Update(elapsedTime);

		if (!m_match->Update(elapsedTime))
		{
			GetApp().Quit();
			return;
		}

		Nz::Time tickDuration = m_match->GetTickDuration();
		if (elapsedTime < tickDuration)
		{
			// Since OS sleep is not that precise, let some time between the wakeup time and the tick
			constexpr Nz::Time wakeUpTime = Nz::Time::Milliseconds(3);

			Nz::Time remainingTime = tickDuration - elapsedTime;
			if (remainingTime > wakeUpTime)
			{
				Nz::Time sleepTime = remainingTime - wakeUpTime;
				std::this_thread::sleep_for(sleepTime.AsDuration<std::chrono::milliseconds>());
			}
		}
	}
}
