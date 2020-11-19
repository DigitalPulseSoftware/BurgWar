// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_STATES_GAME_CONNECTIONSTATE_HPP
#define BURGWAR_STATES_GAME_CONNECTIONSTATE_HPP

#include <ClientLib/ClientSession.hpp>
#include <Client/States/Game/CancelableState.hpp>
#include <Nazara/Network/IpAddress.hpp>
#include <NDK/Widgets/LabelWidget.hpp>
#include <variant>

namespace bw
{
	class LocalSessionManager;

	class ConnectionState final : public CancelableState
	{
		public:
			ConnectionState(std::shared_ptr<StateData> stateData, std::variant<Nz::IpAddress, LocalSessionManager*> remote, std::shared_ptr<AbstractState> previousState);
			~ConnectionState() = default;

		private:
			void HandleConnectionFailure();
			void OnCancelled() override;

			std::shared_ptr<ClientSession> m_clientSession;

			NazaraSlot(ClientSession, OnConnected, m_clientSessionConnectedSlot);
			NazaraSlot(ClientSession, OnDisconnected, m_clientSessionDisconnectedSlot);
	};
}

#include <Client/States/Game/ConnectionState.inl>

#endif
