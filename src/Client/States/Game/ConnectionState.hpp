// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_STATES_STATES_CONNECTIONSTATE_HPP
#define BURGWAR_STATES_STATES_CONNECTIONSTATE_HPP

#include <ClientLib/ClientSession.hpp>
#include <Client/States/AbstractState.hpp>
#include <Nazara/Network/IpAddress.hpp>
#include <NDK/Widgets/LabelWidget.hpp>
#include <variant>

namespace bw
{
	class LocalSessionManager;

	class ConnectionState final : public AbstractState
	{
		public:
			ConnectionState(std::shared_ptr<StateData> stateData, std::variant<Nz::IpAddress, LocalSessionManager*> remote);
			~ConnectionState() = default;

		private:
			void Enter(Ndk::StateMachine& fsm) override;
			void Leave(Ndk::StateMachine& fsm) override;
			bool Update(Ndk::StateMachine& fsm, float elapsedTime) override;
			void UpdateStatus(const std::string& status, const Nz::Color& color);

			std::shared_ptr<AbstractState> m_nextState;
			std::shared_ptr<ClientSession> m_clientSession;
			Ndk::LabelWidget* m_statusLabel;
			float m_nextStateDelay;

			NazaraSlot(ClientSession, OnConnected, m_clientSessionConnectedSlot);
			NazaraSlot(ClientSession, OnDisconnected, m_clientSessionDisconnectedSlot);
	};
}

#include <Client/States/Game/ConnectionState.inl>

#endif
