// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/Game/GameState.hpp>
#include <Client/ClientApp.hpp>
#include <ClientLib/LocalMatch.hpp>

namespace bw
{
	GameState::GameState(std::shared_ptr<StateData> stateDataPtr, std::shared_ptr<ClientSession> clientSession, const Packets::AuthSuccess& authSuccess, const Packets::MatchData& matchData, std::shared_ptr<VirtualDirectory> assetDirectory, std::shared_ptr<VirtualDirectory> scriptDirectory) :
	AbstractState(std::move(stateDataPtr)),
	m_clientSession(std::move(clientSession))
	{
		StateData& stateData = GetStateData();

		m_match = std::make_shared<LocalMatch>(*stateData.app, stateData.window, &stateData.canvas.value(), *m_clientSession, authSuccess, matchData);
		m_match->LoadAssets(std::move(assetDirectory));
		m_match->LoadScripts(std::move(scriptDirectory));

		if (stateData.app->GetConfig().GetBoolOption("Debug.ShowServerGhosts"))
			m_match->InitDebugGhosts();

		const std::string& resourceFolder = stateData.app->GetConfig().GetStringOption("Assets.ResourceFolder");

		if (m_music.OpenFromFile(resourceFolder + "/Music/mushroomdance_0.ogg"))
		{
			m_music.EnableLooping(true);
			m_music.Play();
			m_music.SetVolume(50.f);
		}

		m_clientSession->SendPacket(Packets::Ready{});
	}

	bool GameState::Update(Ndk::StateMachine& fsm, float elapsedTime)
	{
		if (!AbstractState::Update(fsm, elapsedTime))
			return false;

		m_match->Update(elapsedTime);

		return true;
	}
}
