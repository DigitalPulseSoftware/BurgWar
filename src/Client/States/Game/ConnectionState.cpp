// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/Game/ConnectionState.hpp>
#include <CoreLib/NetworkSessionBridge.hpp>
#include <ClientLib/LocalSessionBridge.hpp>
#include <ClientLib/LocalSessionManager.hpp>
#include <Client/ClientApp.hpp>
#include <Client/States/LoginState.hpp>
#include <Client/States/Game/GameState.hpp>
#include <Nazara/Graphics/ColorBackground.hpp>
#include <NDK/Systems/RenderSystem.hpp>
#include <NDK/Widgets/LabelWidget.hpp>
#include <random>

namespace bw
{
	ConnectionState::ConnectionState(std::shared_ptr<StateData> stateData, std::variant<Nz::IpAddress, LocalSessionManager*> remote, std::string playerName) :
	AbstractState(std::move(stateData))
	{
		ClientApp* app = GetStateData().app;
		auto& networkManager = app->GetReactorManager();

		m_statusLabel = CreateWidget<Ndk::LabelWidget>();

		auto CreateMatch = [this](ClientSession& session, const Packets::MatchData& matchData) -> std::shared_ptr<LocalMatch>
		{
			auto gameState = std::make_shared<GameState>(GetStateDataPtr(), m_clientSession, matchData);
			UpdateStatus("Entering game...", Nz::Color::Green * Nz::Color(128, 128, 128));

			m_nextState = gameState;
			m_nextStateDelay = 1.f;

			return gameState->GetMatch();
		};

		m_clientSession = std::make_shared<ClientSession>(*app, CreateMatch, std::move(playerName));

		m_clientSessionConnectedSlot.Connect(m_clientSession->OnConnected, [this] (ClientSession*)
		{
			UpdateStatus("Connected, waiting for match data...", Nz::Color::Green * Nz::Color(128, 128, 128));
		});

		m_clientSessionDisconnectedSlot.Connect(m_clientSession->OnDisconnected, [this](ClientSession*)
		{
			UpdateStatus("Failed to connect to server", Nz::Color::Red);

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

	void ConnectionState::Enter(Ndk::StateMachine& fsm)
	{
		AbstractState::Enter(fsm);

		StateData& stateData = GetStateData();

		stateData.world->GetSystem<Ndk::RenderSystem>().SetDefaultBackground(Nz::ColorBackground::New(Nz::Color(100, 185, 191)));
	}

	void ConnectionState::Leave(Ndk::StateMachine & fsm)
	{
		StateData& stateData = GetStateData();

		stateData.world->GetSystem<Ndk::RenderSystem>().SetDefaultBackground(nullptr);
	}

	bool ConnectionState::Update(Ndk::StateMachine& fsm, float elapsedTime)
	{
		if (m_nextState)
		{
			if ((m_nextStateDelay -= elapsedTime) < 0.f)
			{
				fsm.ChangeState(m_nextState);
				return true;
			}
		}

		m_clientSession->Update();

		return true;
	}

	void ConnectionState::UpdateStatus(const std::string& status, const Nz::Color& color)
	{
		m_statusLabel->UpdateText(Nz::SimpleTextDrawer::Draw(status, 24, 0L, color));
		m_statusLabel->Center();
		m_statusLabel->Show(true);
	}
}
