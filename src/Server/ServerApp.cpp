// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/ServerApp.hpp>
#include <thread>

namespace bw
{
	ServerApp::ServerApp(int argc, char* argv[]) :
	Application(argc, argv),
	BurgApp(LogSide::Server, m_configFile),
	m_configFile(*this)
	{
		if (!m_configFile.LoadFromFile("serverconfig.lua"))
			throw std::runtime_error("failed to load config file");

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
		matchSettings.tickDuration = 1.f / tickRate;

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

	int ServerApp::Run()
	{
		Nz::Clock updateClock;
		Nz::UInt64 tickDuration = static_cast<Nz::UInt64>(m_match->GetTickDuration() * 1'000'000);

		while (Application::Run())
		{
			BurgApp::Update();

			if (!m_match->Update(GetUpdateTime()))
				break;

			Nz::UInt64 elapsedTime = updateClock.Restart();
			if (tickDuration > elapsedTime)
			{
				// Since OS sleep is not that precise, let some time between the wakeup time and the tick
				constexpr Nz::UInt64 wakeUpTime = 3'000;

				Nz::UInt64 remainingTime = tickDuration - elapsedTime;
				if (remainingTime > wakeUpTime)
				{
					Nz::UInt64 sleepTime = remainingTime - wakeUpTime;
					std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime / 1'000));
				}
			}
		}

		return 0;
	}
	
	void ServerApp::Quit()
	{
		Application::Quit();
	}
}
