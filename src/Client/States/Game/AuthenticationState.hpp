// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_STATES_GAME_AUTHENTICATIONSTATE_HPP
#define BURGWAR_STATES_GAME_AUTHENTICATIONSTATE_HPP

#include <ClientLib/ClientSession.hpp>
#include <Client/States/Game/StatusState.hpp>
#include <NDK/Widgets/LabelWidget.hpp>

namespace bw
{
	class AuthenticationState final : public StatusState
	{
		public:
			AuthenticationState(std::shared_ptr<StateData> stateData, std::shared_ptr<ClientSession> clientSession, std::string playerName);
			~AuthenticationState() = default;

		private:
			void Enter(Ndk::StateMachine& fsm) override;
			bool Update(Ndk::StateMachine& fsm, float elapsedTime) override;

			std::shared_ptr<AbstractState> m_nextState;
			std::shared_ptr<ClientSession> m_clientSession;
			std::string m_playerName;
			float m_nextStateDelay;

			NazaraSlot(ClientSession, OnDisconnected, m_onDisconnectedSlot);
			NazaraSlot(ClientSession, OnAuthFailure, m_onAuthFailedSlot);
			NazaraSlot(ClientSession, OnAuthSuccess, m_onAuthSucceededSlot);
			NazaraSlot(ClientSession, OnMatchData, m_onMatchDataSlot);
	};
}

#include <Client/States/Game/AuthenticationState.inl>

#endif
