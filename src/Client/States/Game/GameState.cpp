// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/Game/GameState.hpp>
#include <ClientLib/ClientMatch.hpp>
#include <Client/ClientApp.hpp>
#include <Client/States/BackgroundState.hpp>
#include <Client/States/MainMenuState.hpp>

namespace bw
{
	GameState::GameState(std::shared_ptr<StateData> stateDataPtr, std::shared_ptr<ClientSession> clientSession, const Packets::AuthSuccess& authSuccess, const Packets::MatchData& matchData, std::shared_ptr<Nz::VirtualDirectory> assetDirectory, std::shared_ptr<Nz::VirtualDirectory> scriptDirectory) :
	AbstractState(std::move(stateDataPtr)),
	m_clientSession(std::move(clientSession))
	{
		StateData& stateData = GetStateData();

		m_match = std::make_shared<ClientMatch>(*stateData.app, stateData.window, stateData.window, &stateData.canvas.value(), *m_clientSession, authSuccess, matchData);
		m_match->LoadAssets(std::move(assetDirectory));
		m_match->LoadScripts(std::move(scriptDirectory));

		if (stateData.app->GetConfig().GetBoolValue("Debug.ShowServerGhosts"))
			m_match->InitDebugGhosts();

		m_clientSession->SendPacket(Packets::Ready{});
	}

	void GameState::Leave(Ndk::StateMachine& /*fsm*/)
	{
		if (m_clientSession)
			m_clientSession->Disconnect();
	}

	bool GameState::Update(Ndk::StateMachine& fsm, Nz::Time elapsedTime)
	{
		if (!AbstractState::Update(fsm, elapsedTime))
			return false;

		if (!m_match->Update(elapsedTime))
		{
			fsm.ResetState(std::make_shared<BackgroundState>(GetStateDataPtr()));
			fsm.PushState(std::make_shared<MainMenuState>(GetStateDataPtr()));
			return true;
		}

		return true;
	}
}
