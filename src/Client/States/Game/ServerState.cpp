// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/Game/ServerState.hpp>
#include <CoreLib/Utils.hpp>
#include <ClientLib/LocalSessionManager.hpp>
#include <Client/ClientApp.hpp>
#include <Client/States/BackgroundState.hpp>
#include <Client/States/Game/ConnectionState.hpp>

namespace bw
{
	ServerState::ServerState(std::shared_ptr<StateData> stateDataPtr, Match::MatchSettings matchSettings, Match::GamemodeSettings gamemodeSettings, Match::ModSettings modSettings, std::shared_ptr<AbstractState> originalState) :
	AbstractState(std::move(stateDataPtr)),
	m_originalState(std::move(originalState))
	{
		ClientApp& app = *GetStateData().app;
		const ConfigFile& config = app.GetConfig();

		m_match.emplace(app, std::move(matchSettings), std::move(gamemodeSettings), std::move(modSettings));

		MatchSessions& sessions = m_match->GetSessions();
		m_localSessionManager = sessions.CreateSessionManager<LocalSessionManager>();

		if (config.GetBoolValue("Debug.SendServerState"))
			m_match->InitDebugGhosts();
	}

	void ServerState::Enter(Ndk::StateMachine& fsm)
	{
		fsm.PushState(std::make_shared<ConnectionState>(GetStateDataPtr(), m_localSessionManager, m_originalState));
	}

	bool ServerState::Update(Ndk::StateMachine& fsm, float elapsedTime)
	{
		if (!AbstractState::Update(fsm, elapsedTime))
			return false;

		if (!m_match->Update(elapsedTime))
		{
			fsm.ResetState(std::make_shared<BackgroundState>(GetStateDataPtr()));
			fsm.PushState(m_originalState);
			return true;
		}

		return true;
	}
}
