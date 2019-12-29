// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/Game/AuthenticationState.hpp>
#include <Client/ClientApp.hpp>
#include <Client/States/LoginState.hpp>
#include <Client/States/Game/AssetDownloadState.hpp>

namespace bw
{
	AuthenticationState::AuthenticationState(std::shared_ptr<StateData> stateData, std::shared_ptr<ClientSession> clientSession, std::string playerName) :
	StatusState(std::move(stateData)),
	m_clientSession(std::move(clientSession)),
	m_playerName(std::move(playerName))
	{
		m_onAuthFailedSlot.Connect(m_clientSession->OnAuthFailure, [this](ClientSession*, const Packets::AuthFailure& /*data*/)
		{
			UpdateStatus("Failed to authenticate", Nz::Color::Red);

			m_nextState = std::make_shared<LoginState>(GetStateDataPtr());
			m_nextStateDelay = 3.f;
		});

		m_onAuthSucceededSlot.Connect(m_clientSession->OnAuthSuccess, [this](ClientSession*, const Packets::AuthSuccess& /*data*/)
		{
			UpdateStatus("Authentication succeeded, waiting for match data...", Nz::Color::White);
		});

		m_onMatchDataSlot.Connect(m_clientSession->OnMatchData, [this](ClientSession*, const Packets::MatchData& data)
		{
			UpdateStatus("Received match data", Nz::Color::White);

			m_nextState = std::make_shared<AssetDownloadState>(GetStateDataPtr(), m_clientSession, data);
			m_nextStateDelay = 0.5f;
		});
	}

	void AuthenticationState::Enter(Ndk::StateMachine& fsm)
	{
		StatusState::Enter(fsm);

		Packets::Auth authPacket;
		authPacket.players.emplace_back().nickname = std::move(m_playerName);

		m_clientSession->SendPacket(std::move(authPacket));
	}

	bool AuthenticationState::Update(Ndk::StateMachine& fsm, float elapsedTime)
	{
		if (!StatusState::Update(fsm, elapsedTime))
			return false;

		if (m_nextState)
		{
			if ((m_nextStateDelay -= elapsedTime) < 0.f)
			{
				fsm.ChangeState(m_nextState);
				return true;
			}
		}

		return true;
	}
}
