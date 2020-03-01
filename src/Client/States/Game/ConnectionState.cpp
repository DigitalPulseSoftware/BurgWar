// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/Game/ConnectionState.hpp>
#include <CoreLib/NetworkSessionBridge.hpp>
#include <ClientLib/LocalSessionBridge.hpp>
#include <ClientLib/LocalSessionManager.hpp>
#include <Client/ClientApp.hpp>
#include <Client/States/BackgroundState.hpp>
#include <Client/States/LoginState.hpp>
#include <Client/States/Game/AuthenticationState.hpp>
#include <Client/States/Game/ConnectedState.hpp>
#include <random>

namespace bw
{
	ConnectionState::ConnectionState(std::shared_ptr<StateData> stateData, std::variant<Nz::IpAddress, LocalSessionManager*> remote, std::string playerName) :
	StatusState(std::move(stateData)),
	m_failed(false)
	{
		ClientApp* app = GetStateData().app;
		auto& networkManager = app->GetReactorManager();

		m_clientSession = std::make_shared<ClientSession>(*app);

		m_clientSessionConnectedSlot.Connect(m_clientSession->OnConnected, [this, playerName] (ClientSession*)
		{
			UpdateStatus("Connected, authenticating...", Nz::Color::White);

			auto authState = std::make_shared<AuthenticationState>(GetStateDataPtr(), m_clientSession, playerName);

			m_nextState = std::make_shared<ConnectedState>(GetStateDataPtr(), m_clientSession, std::move(authState));
			m_nextStateDelay = 0.5f;
		});

		m_clientSessionDisconnectedSlot.Connect(m_clientSession->OnDisconnected, [this](ClientSession*)
		{
			UpdateStatus("Failed to connect to server", Nz::Color::Red);

			m_failed = true;
			m_nextState = std::make_shared<LoginState>(GetStateDataPtr());
			m_nextStateDelay = 3.f;
		});

		std::visit([&](auto&& value)
		{
			using T = std::decay_t<decltype(value)>;

			if constexpr (std::is_same_v<T, Nz::IpAddress>)
			{
				m_clientSession->Connect(networkManager.ConnectToServer(value, 0));
				UpdateStatus("Connecting to " + value.ToString().ToStdString() + "...", Nz::Color::White);
			}
			else if constexpr (std::is_same_v<T, LocalSessionManager*>)
			{
				m_clientSession->Connect(value->CreateSession());
			}
			else
				static_assert(AlwaysFalse<T>::value, "non-exhaustive visitor");
		}, remote);
	}

	bool ConnectionState::Update(Ndk::StateMachine& fsm, float elapsedTime)
	{
		if (!AbstractState::Update(fsm, elapsedTime))
			return false;

		if (m_nextState)
		{
			if ((m_nextStateDelay -= elapsedTime) < 0.f)
			{
				if (m_failed)
					fsm.PushState(std::make_shared<BackgroundState>(GetStateDataPtr()));

				fsm.ChangeState(m_nextState);
				return true;
			}
		}

		return true;
	}
}
