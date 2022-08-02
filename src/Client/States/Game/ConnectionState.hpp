// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_STATES_GAME_CONNECTIONSTATE_HPP
#define BURGWAR_STATES_GAME_CONNECTIONSTATE_HPP

#include <ClientLib/ClientSession.hpp>
#include <Client/States/Game/CancelableState.hpp>
#include <Nazara/Network/IpAddress.hpp>
#include <Nazara/Widgets/LabelWidget.hpp>
#include <tl/expected.hpp>
#include <optional>
#include <thread>
#include <variant>
#include <vector>

namespace bw
{
	class LocalSessionManager;

	class ConnectionState final : public CancelableState
	{
		public:
			struct ServerName
			{
				std::string hostname;
				Nz::UInt16 port;
			};

			using Address = std::variant<ServerName, Nz::IpAddress, LocalSessionManager*>;
			using AddressList = std::vector<Address>;

			ConnectionState(std::shared_ptr<StateData> stateData, Address remoteAddress, std::shared_ptr<AbstractState> previousState);
			ConnectionState(std::shared_ptr<StateData> stateData, AddressList remoteAddresses, std::shared_ptr<AbstractState> previousState);
			~ConnectionState();

		private:
			void HandleConnectionFailure();
			void OnCancelled() override;
			void ProcessNextAddress();
			void ProcessNextAddress(const ServerName& name);
			void ProcessNextAddress(const Nz::IpAddress& address);
			void ProcessNextAddress(LocalSessionManager* sessionManager);
			bool Update(Ndk::StateMachine& fsm, float elapsedTime) override;

			struct ResolvingData
			{
				std::atomic_bool hasResult = false;
				std::thread thread;
				tl::expected<std::vector<Nz::IpAddress>, std::string /*error*/> result;
				ServerName serverName;
			};

			std::optional<ResolvingData> m_resolvingData;
			std::shared_ptr<ClientSession> m_clientSession;
			std::size_t m_currentAddressIndex;
			AddressList m_addresses;
			float m_timeBeforeGivingUp;

			NazaraSlot(ClientSession, OnConnected, m_clientSessionConnectedSlot);
			NazaraSlot(ClientSession, OnDisconnected, m_clientSessionDisconnectedSlot);
	};
}

#include <Client/States/Game/ConnectionState.inl>

#endif
