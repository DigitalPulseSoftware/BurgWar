// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/Game/ConnectionState.hpp>
#include <CoreLib/NetworkSessionBridge.hpp>
#include <ClientLib/LocalSessionBridge.hpp>
#include <ClientLib/LocalSessionManager.hpp>
#include <Client/ClientApp.hpp>
#include <Client/States/BackgroundState.hpp>
#include <Client/States/MainMenuState.hpp>
#include <Client/States/Game/AuthenticationState.hpp>
#include <Client/States/Game/ConnectedState.hpp>
#include <random>

namespace bw
{
	ConnectionState::ConnectionState(std::shared_ptr<StateData> stateData, std::variant<Nz::IpAddress, LocalSessionManager*> remote) :
	StatusState(std::move(stateData))
	{
		ClientApp* app = GetStateData().app;
		auto& networkManager = app->GetReactorManager();

		m_clientSession = std::make_shared<ClientSession>(*app);

		m_clientSessionConnectedSlot.Connect(m_clientSession->OnConnected, [this] (ClientSession*)
		{
			UpdateStatus("Connected, authenticating...", Nz::Color::White);

			auto authState = std::make_shared<AuthenticationState>(GetStateDataPtr(), m_clientSession);

			m_nextStateCallback = [this, authState = std::move(authState)](Ndk::StateMachine& fsm)
			{
				fsm.ChangeState(std::make_shared<ConnectedState>(GetStateDataPtr(), m_clientSession, std::move(authState)));
			};
			m_nextStateDelay = 0.5f;
		});

		m_clientSessionDisconnectedSlot.Connect(m_clientSession->OnDisconnected, [this](ClientSession*)
		{
			HandleConnectionFailure();
		});

		std::visit([&](auto&& value)
		{
			using T = std::decay_t<decltype(value)>;

			if constexpr (std::is_same_v<T, Nz::IpAddress>)
			{
				auto sessionBridge = networkManager.ConnectToServer(value, 0);
				if (sessionBridge)
				{
					m_clientSession->Connect(std::move(sessionBridge));

					UpdateStatus("Connecting to " + value.ToString().ToStdString() + "...", Nz::Color::White);
				}
				else
					HandleConnectionFailure();
			}
			else if constexpr (std::is_same_v<T, LocalSessionManager*>)
			{
				m_clientSession->Connect(value->CreateSession());
			}
			else
				static_assert(AlwaysFalse<T>::value, "non-exhaustive visitor");
		}, remote);
	}

	void ConnectionState::HandleConnectionFailure()
	{
		UpdateStatus("Failed to connect to server", Nz::Color::Red);

		m_nextStateCallback = [this](Ndk::StateMachine& fsm)
		{
			fsm.ResetState(std::make_shared<BackgroundState>(GetStateDataPtr()));
			fsm.PushState(std::make_shared<MainMenuState>(GetStateDataPtr()));
		};

		m_nextStateDelay = 3.f;
	}

	bool ConnectionState::Update(Ndk::StateMachine& fsm, float elapsedTime)
	{
		if (!AbstractState::Update(fsm, elapsedTime))
			return false;

		if (m_nextStateCallback)
		{
			if ((m_nextStateDelay -= elapsedTime) < 0.f)
			{
				m_nextStateCallback(fsm);
				return true;
			}
		}

		return true;
	}
}
