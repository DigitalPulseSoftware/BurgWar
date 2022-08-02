// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/JoinServerState.hpp>
#include <CoreLib/ConfigFile.hpp>
#include <Client/ClientApp.hpp>
#include <Client/States/OptionState.hpp>
#include <Client/States/Game/ConnectionState.hpp>
#include <Client/States/Game/ServerState.hpp>
#include <Nazara/Core/String.hpp>
#include <Nazara/Renderer/DebugDrawer.hpp>
#include <Nazara/Renderer/Renderer.hpp>
#include <Nazara/Utility/SimpleTextDrawer.hpp>
#include <NDK/StateMachine.hpp>
#include <Nazara/Widgets/CheckboxWidget.hpp>
#include <Nazara/Widgets/LabelWidget.hpp>
#include <Nazara/Widgets/TextAreaWidget.hpp>
#include <cassert>
#include <chrono>

namespace bw
{
	JoinServerState::JoinServerState(std::shared_ptr<StateData> stateData, std::shared_ptr<AbstractState> previousState) :
	AbstractState(std::move(stateData)),
	m_previousState(std::move(previousState))
	{
		m_statusLabel = CreateWidget<Ndk::LabelWidget>();
		m_statusLabel->Hide();

		m_serverLabel = CreateWidget<Ndk::LabelWidget>();
		m_serverLabel->UpdateText(Nz::SimpleTextDrawer::Draw("Server: ", 24));

		m_serverAddressLayout = CreateWidget<Ndk::BoxLayout>(Ndk::BoxLayoutOrientation_Horizontal);

		m_serverAddressArea = m_serverAddressLayout->Add<Ndk::TextAreaWidget>();
		m_serverAddressArea->EnableBackground(true);
		m_serverAddressArea->SetBackgroundColor(Nz::Color::White);
		m_serverAddressArea->Resize({ 400.f, 36.f });
		m_serverAddressArea->SetMaximumWidth(400.f);
		m_serverAddressArea->SetTextColor(Nz::Color::Black);

		m_serverPortArea = m_serverAddressLayout->Add<Ndk::TextAreaWidget>();
		m_serverPortArea->EnableBackground(true);
		m_serverPortArea->SetBackgroundColor(Nz::Color::White);
		m_serverPortArea->Resize({ 50.f, 36.f });
		m_serverPortArea->SetMaximumWidth(100.f);
		m_serverPortArea->SetTextColor(Nz::Color::Black);
		m_serverPortArea->SetCharacterFilter([](Nz::UInt32 character) 
		{
			if (character < U'0' || character > U'9')
				return false;

			return true;
		});


		m_connectionButton = CreateWidget<Ndk::ButtonWidget>();
		m_connectionButton->UpdateText(Nz::SimpleTextDrawer::Draw("Connect to server", 24));
		m_connectionButton->Resize(m_connectionButton->GetPreferredSize());

		m_connectionButton->OnButtonTrigger.Connect([this](const Ndk::ButtonWidget*)
		{
			OnConnectionPressed();
		});
		
		m_backButton = CreateWidget<Ndk::ButtonWidget>();
		m_backButton->UpdateText(Nz::SimpleTextDrawer::Draw("Back", 24));
		m_backButton->Resize(m_backButton->GetPreferredSize());
		
		m_backButton->OnButtonTrigger.Connect([this](const Ndk::ButtonWidget*)
		{
			OnBackPressed();
		});

		const ConfigFile& playerConfig = GetStateData().app->GetPlayerSettings();

		m_serverAddressArea->SetText(playerConfig.GetStringValue("JoinServer.Address"));
		m_serverPortArea->SetText(std::to_string(playerConfig.GetIntegerValue<Nz::UInt16>("JoinServer.Port")));
	}

	void JoinServerState::Leave(Ndk::StateMachine& fsm)
	{
		m_statusLabel->Hide();

		AbstractState::Leave(fsm);
	}

	bool JoinServerState::Update(Ndk::StateMachine& fsm, float elapsedTime)
	{
		if (!AbstractState::Update(fsm, elapsedTime))
			return false;

		if (m_nextGameState)
			fsm.ResetState(std::move(m_nextGameState));
		else if (m_nextState)
			fsm.ChangeState(std::move(m_nextState));

		return true;
	}

	void JoinServerState::OnBackPressed()
	{
		m_nextState = std::move(m_previousState);
	}

	void JoinServerState::OnConnectionPressed()
	{
		std::string serverHostname = m_serverAddressArea->GetText();
		if (serverHostname.empty())
		{
			UpdateStatus("Error: blank server address", Nz::Color::Red);
			return;
		}

		Nz::String serverPort = m_serverPortArea->GetText();
		if (serverPort.IsEmpty())
		{
			UpdateStatus("Error: blank server port", Nz::Color::Red);
			return;
		}

		long long rawPort;
		if (!serverPort.ToInteger(&rawPort) || rawPort <= 0 || rawPort > 0xFFFF)
		{
			UpdateStatus("Error: " + serverPort + " is not a valid port", Nz::Color::Red);
			return;
		}

		ConfigFile& playerConfig = GetStateData().app->GetPlayerSettings();
		playerConfig.SetStringValue("JoinServer.Address", serverHostname);
		playerConfig.SetIntegerValue("JoinServer.Port", rawPort);

		GetStateData().app->SavePlayerConfig();

		ConnectionState::ServerName address;
		address.hostname = serverHostname;
		address.port = static_cast<Nz::UInt16>(rawPort);

		m_nextGameState = std::make_shared<ConnectionState>(GetStateDataPtr(), std::move(address), shared_from_this());
	}

	void JoinServerState::LayoutWidgets()
	{
		Nz::Vector2f canvasSize = GetStateData().canvas->GetSize();
		Nz::Vector2f center = canvasSize / 2.f;

		constexpr float padding = 10.f;

		std::array<Nz::BaseWidget*, 3> widgets = {
			m_statusLabel,
			m_serverAddressLayout,
			m_connectionButton
		};

		float totalSize = padding * (widgets.size() - 1);
		for (Nz::BaseWidget* widget : widgets)
			totalSize += widget->GetSize().y;

		Nz::Vector2f cursor = center;
		cursor.y -= totalSize / 2.f;

		m_statusLabel->SetPosition({ 0.f, cursor.y, 0.f });
		m_statusLabel->CenterHorizontal();
		cursor.y += m_statusLabel->GetSize().y + padding;

		m_serverAddressLayout->Resize({ 500.f, 36.f }); //< Force box layout (FIXME)

		m_serverAddressLayout->SetPosition({ 0.f, cursor.y, 0.f });
		m_serverAddressLayout->CenterHorizontal();
		cursor.y += m_serverAddressLayout->GetSize().y + padding;

		m_serverLabel->SetPosition(m_serverAddressLayout->GetPosition() - Nz::Vector2f(m_serverLabel->GetSize().x, 0.f));

		m_connectionButton->SetPosition({ 0.f, cursor.y, 0.f });
		m_connectionButton->CenterHorizontal();
		cursor.y += m_connectionButton->GetSize().y + padding;

		m_backButton->SetPosition({ 0.f, cursor.y, 0.f });
		m_backButton->CenterHorizontal();
		cursor.y += m_backButton->GetSize().y + padding;
	}

	void JoinServerState::UpdateStatus(const std::string& status, const Nz::Color& color)
	{
		m_statusLabel->UpdateText(Nz::SimpleTextDrawer::Draw(status, 24, 0L, color));
		m_statusLabel->Resize(m_statusLabel->GetPreferredSize()); //< FIXME
		m_statusLabel->CenterHorizontal();
		m_statusLabel->Show(true);
	}
}
