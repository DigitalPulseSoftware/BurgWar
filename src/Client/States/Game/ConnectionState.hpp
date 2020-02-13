// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_STATES_GAME_CONNECTIONSTATE_HPP
#define BURGWAR_STATES_GAME_CONNECTIONSTATE_HPP

#include <ClientLib/ClientSession.hpp>
#include <Client/States/Game/StatusState.hpp>
#include <Nazara/Network/IpAddress.hpp>
#include <NDK/Widgets/LabelWidget.hpp>
#include <variant>

namespace bw
{
	class LocalSessionManager;

	class ConnectionState final : public StatusState
	{
		public:
			ConnectionState(std::shared_ptr<StateData> stateData, std::variant<Nz::IpAddress, LocalSessionManager*> remote, std::string playerName);
			~ConnectionState() = default;

		private:
			bool Update(Ndk::StateMachine& fsm, float elapsedTime) override;

			std::shared_ptr<AbstractState> m_nextState;
			std::shared_ptr<ClientSession> m_clientSession;
			float m_nextStateDelay;

			NazaraSlot(ClientSession, OnConnected, m_clientSessionConnectedSlot);
			NazaraSlot(ClientSession, OnDisconnected, m_clientSessionDisconnectedSlot);
	};
}

#include <Client/States/Game/ConnectionState.inl>

#endif
