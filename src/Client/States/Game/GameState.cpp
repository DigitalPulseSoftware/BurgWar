// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/Game/GameState.hpp>
#include <Client/ClientApp.hpp>
#include <Client/States/BackgroundState.hpp>
#include <Client/States/Game/ConnectionLostState.hpp>
#include <ClientLib/LocalMatch.hpp>
#include <random>

namespace bw
{
	GameState::GameState(std::shared_ptr<StateData> stateDataPtr, std::shared_ptr<ClientSession> clientSession, const Packets::MatchData& matchData, std::shared_ptr<VirtualDirectory> assetDirectory, std::shared_ptr<VirtualDirectory> scriptDirectory) :
	AbstractState(std::move(stateDataPtr)),
	m_clientSession(std::move(clientSession))
	{
		StateData& stateData = GetStateData();

		m_match = std::make_shared<LocalMatch>(*stateData.app, stateData.window, &stateData.canvas.value(), *m_clientSession, matchData);
		m_match->LoadAssets(std::move(assetDirectory));
		m_match->LoadScripts(std::move(scriptDirectory));

		m_clientSession->SendPacket(Packets::Ready{});
	}

	void GameState::Enter(Ndk::StateMachine& fsm)
	{
		AbstractState::Enter(fsm);
	}

	bool GameState::Update(Ndk::StateMachine& fsm, float elapsedTime)
	{
		if (!m_clientSession->IsConnected())
		{
			fsm.ResetState(std::make_shared<BackgroundState>(GetStateDataPtr()));
			fsm.PushState(std::make_shared<ConnectionLostState>(GetStateDataPtr()));
			return true;
		}

		m_match->Update(elapsedTime);

		return true;
	}
}
