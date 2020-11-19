// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_STATES_GAME_AUTHENTICATIONSTATE_HPP
#define BURGWAR_STATES_GAME_AUTHENTICATIONSTATE_HPP

#include <CoreLib/Protocol/Packets.hpp>
#include <ClientLib/ClientSession.hpp>
#include <Client/States/Game/CancelableState.hpp>
#include <NDK/Widgets/LabelWidget.hpp>
#include <optional>

namespace bw
{
	class AuthenticationState final : public CancelableState
	{
		public:
			AuthenticationState(std::shared_ptr<StateData> stateData, std::shared_ptr<ClientSession> clientSession, std::shared_ptr<AbstractState> originalState);
			~AuthenticationState() = default;

		private:
			void Enter(Ndk::StateMachine& fsm) override;
			void OnCancelled() override;

			std::optional<Packets::AuthSuccess> m_authSuccessPacket;
			std::shared_ptr<ClientSession> m_clientSession;

			NazaraSlot(ClientSession, OnAuthFailure, m_onAuthFailedSlot);
			NazaraSlot(ClientSession, OnAuthSuccess, m_onAuthSucceededSlot);
			NazaraSlot(ClientSession, OnMatchData, m_onMatchDataSlot);
	};
}

#include <Client/States/Game/AuthenticationState.inl>

#endif
