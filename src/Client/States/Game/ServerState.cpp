// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/Game/ServerState.hpp>
#include <CoreLib/NetworkSessionManager.hpp>
#include <CoreLib/Utils.hpp>
#include <ClientLib/LocalSessionManager.hpp>
#include <Client/ClientApp.hpp>
#include <Client/States/Game/ConnectionState.hpp>

namespace bw
{
	ServerState::ServerState(std::shared_ptr<StateData> stateDataPtr, Nz::UInt16 listenPort, const std::string& gamemode, const std::string& map, std::shared_ptr<AbstractState> originalState) :
	AbstractState(std::move(stateDataPtr)),
	m_originalState(std::move(originalState))
	{
		ClientApp& app = *GetStateData().app;
		const ConfigFile& config = app.GetConfig();

		Match::GamemodeSettings gamemodeSettings;
		gamemodeSettings.name = gamemode;
		gamemodeSettings.properties.emplace("respawntime", PropertySingleValue<PropertyType::Integer>(2));

		Match::MatchSettings matchSettings;
		matchSettings.maxPlayerCount = 64;
		matchSettings.name = "local";
		matchSettings.tickDuration = 1.f / config.GetFloatValue<float>("GameSettings.TickRate");

		// Load map
		if (!EndsWith(map, ".bmap"))
		{
			if (std::filesystem::is_directory(map))
				matchSettings.map = Map::LoadFromDirectory(map);
			else
				matchSettings.map = Map::LoadFromBinary(map + ".bmap");
		}
		else
			matchSettings.map = Map::LoadFromBinary(map);

		m_match.emplace(app, std::move(matchSettings), std::move(gamemodeSettings));

		MatchSessions& sessions = m_match->GetSessions();
		m_localSessionManager = sessions.CreateSessionManager<LocalSessionManager>();
		if (listenPort != 0)
			m_networkSessionManager = sessions.CreateSessionManager<NetworkSessionManager>(listenPort, 64);
		else
			m_networkSessionManager = nullptr;

		if (config.GetBoolValue("Debug.SendServerState"))
			m_match->InitDebugGhosts();
	}

	void ServerState::Enter(Ndk::StateMachine& fsm)
	{
		fsm.PushState(std::make_shared<ConnectionState>(GetStateDataPtr(), m_localSessionManager, m_originalState));
		/*Nz::IpAddress serverAddress = Nz::IpAddress::LoopbackIpV4;
		serverAddress.SetPort(14768);

		fsm.PushState(std::make_shared<ConnectionState>(GetStateDataPtr(), serverAddress));*/
	}

	bool ServerState::Update(Ndk::StateMachine& fsm, float elapsedTime)
	{
		if (!AbstractState::Update(fsm, elapsedTime))
			return false;

		m_match->Update(elapsedTime);
		return true;
	}
}
