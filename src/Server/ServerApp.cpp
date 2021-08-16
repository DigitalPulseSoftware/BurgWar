// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Server/ServerApp.hpp>
#include <CoreLib/NetworkSessionManager.hpp>
#include <Nazara/Core/Thread.hpp>

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

		Match::GamemodeSettings gamemodeSettings;
		gamemodeSettings.name = m_configFile.GetStringValue("ServerSettings.Gamemode");

		Match::MatchSettings matchSettings;
		matchSettings.map = Map::LoadFromBinary(m_configFile.GetStringValue("ServerSettings.MapFile"));
		matchSettings.maxPlayerCount = 64;
		matchSettings.name = "local";
		matchSettings.tickDuration = 1.f / m_configFile.GetFloatValue<float>("ServerSettings.TickRate");

		Match::ModSettings modSettings;

		// FIXME: Allow to select enabled mods
		for (auto&& [modId, mod] : GetMods())
			modSettings.enabledMods[modId] = Match::ModSettings::ModEntry{};

		m_match = std::make_unique<Match>(*this, std::move(matchSettings), std::move(gamemodeSettings), std::move(modSettings));
		m_match->GetSessions().CreateSessionManager<NetworkSessionManager>(Nz::UInt16(14768), 64);
	}

	int ServerApp::Run()
	{
		while (Application::Run())
		{
			BurgApp::Update();

			m_match->Update(GetUpdateTime());

			//TODO: Sleep only when server is not overloaded
			Nz::Thread::Sleep(1);
		}

		return 0;
	}
}
