// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/Game/AuthenticationState.hpp>
#include <Client/ClientAppComponent.hpp>
#include <Client/States/MainMenuState.hpp>
#include <Client/States/Game/ResourceDownloadState.hpp>

namespace bw
{
	AuthenticationState::AuthenticationState(std::shared_ptr<StateData> stateData, std::shared_ptr<ClientSession> clientSession, std::shared_ptr<AbstractState> originalState) :
	CancelableState(std::move(stateData), std::move(originalState)),
	m_clientSession(std::move(clientSession))
	{
		m_onAuthFailedSlot.Connect(m_clientSession->OnAuthFailure, [this](ClientSession*, const Packets::AuthFailure& /*data*/)
		{
			UpdateStatus("Failed to authenticate", Nz::Color::Red());
			Cancel(Nz::Time::Seconds(3));
		});

		m_onAuthSucceededSlot.Connect(m_clientSession->OnAuthSuccess, [this](ClientSession*, const Packets::AuthSuccess& data)
		{
			UpdateStatus("Authentication succeeded, waiting for match data...", Nz::Color::White());
			m_authSuccessPacket = data;
		});

		m_onMatchDataSlot.Connect(m_clientSession->OnMatchData, [this](ClientSession*, const Packets::MatchData& data)
		{
			if (!m_authSuccessPacket)
			{
				UpdateStatus("Protocol error", Nz::Color::Red());
				Cancel(Nz::Time::Seconds(3));
				return;
			}

			UpdateStatus("Received match data", Nz::Color::White());

			SwitchToState(std::make_shared<ResourceDownloadState>(GetStateDataPtr(), m_clientSession, m_authSuccessPacket.value(), data, GetOriginalState()), Nz::Time::Seconds(0.5f));
		});
	}

	void AuthenticationState::Enter(Nz::StateMachine& fsm)
	{
		StatusState::Enter(fsm);

		ConfigFile& playerConfig = GetStateData().appComponent->GetPlayerSettings();

		Packets::Auth authPacket;
		authPacket.players.emplace_back().nickname = playerConfig.GetStringValue("Player.Name");

		m_clientSession->SendPacket(std::move(authPacket));
	}

	void AuthenticationState::OnCancelled()
	{
		m_clientSession->Disconnect();
	}
}
