// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/StartServerState.hpp>
#include <CoreLib/ConfigFile.hpp>
#include <Client/ClientApp.hpp>
#include <Client/States/OptionState.hpp>
#include <Client/States/Game/ConnectionState.hpp>
#include <Client/States/Game/ServerState.hpp>
#include <Nazara/Core/String.hpp>
#include <Nazara/Network/Algorithm.hpp>
#include <Nazara/Network/IpAddress.hpp>
#include <Nazara/Renderer/DebugDrawer.hpp>
#include <Nazara/Renderer/Renderer.hpp>
#include <Nazara/Utility/SimpleTextDrawer.hpp>
#include <NDK/StateMachine.hpp>
#include <NDK/Widgets/CheckboxWidget.hpp>
#include <NDK/Widgets/LabelWidget.hpp>
#include <NDK/Widgets/TextAreaWidget.hpp>
#include <cassert>
#include <chrono>

namespace bw
{
	StartServerState::StartServerState(std::shared_ptr<StateData> stateData, std::shared_ptr<AbstractState> previousState) :
	AbstractState(std::move(stateData)),
	m_previousState(std::move(previousState))
	{
		m_statusLabel = CreateWidget<Ndk::LabelWidget>();
		m_statusLabel->Hide();

		m_serverConfigLayout = CreateWidget<Ndk::BoxLayout>(Ndk::BoxLayoutOrientation_Vertical);
		m_serverConfigLayout->EnableBackground(true);
		m_serverConfigLayout->SetBackgroundColor(Nz::Color(0, 0, 0, 100));
		m_serverConfigLayout->Resize({ 400.f, 250.f });

		Ndk::LabelWidget* configurationLabel = m_serverConfigLayout->Add<Ndk::LabelWidget>();
		configurationLabel->UpdateText(Nz::SimpleTextDrawer::Draw("Server configuration", 36));

		Ndk::BoxLayout* gamemodeLayout = m_serverConfigLayout->Add<Ndk::BoxLayout>(Ndk::BoxLayoutOrientation_Horizontal);

		Ndk::LabelWidget* gamemodeLabel = gamemodeLayout->Add<Ndk::LabelWidget>();
		gamemodeLabel->UpdateText(Nz::SimpleTextDrawer::Draw("Gamemode: ", 24));

		m_gamemodeArea = gamemodeLayout->Add<Ndk::TextAreaWidget>();
		m_gamemodeArea->EnableBackground(true);
		m_gamemodeArea->SetBackgroundColor(Nz::Color::White);
		m_gamemodeArea->SetTextColor(Nz::Color::Black);

		Ndk::BoxLayout* mapLayout = m_serverConfigLayout->Add<Ndk::BoxLayout>(Ndk::BoxLayoutOrientation_Horizontal);

		Ndk::LabelWidget* mapLabel = mapLayout->Add<Ndk::LabelWidget>();
		mapLabel->UpdateText(Nz::SimpleTextDrawer::Draw("Map: ", 24));

		m_mapArea = mapLayout->Add<Ndk::TextAreaWidget>();
		m_mapArea->EnableBackground(true);
		m_mapArea->SetBackgroundColor(Nz::Color::White);
		m_mapArea->SetTextColor(Nz::Color::Black);

		Ndk::BoxLayout* portLayout = m_serverConfigLayout->Add<Ndk::BoxLayout>(Ndk::BoxLayoutOrientation_Horizontal);

		Ndk::LabelWidget* portLabel = portLayout->Add<Ndk::LabelWidget>();
		portLabel->UpdateText(Nz::SimpleTextDrawer::Draw("Port: ", 24));

		m_portArea = portLayout->Add<Ndk::TextAreaWidget>();
		m_portArea->EnableBackground(true);
		m_portArea->SetBackgroundColor(Nz::Color::White);
		m_portArea->SetTextColor(Nz::Color::Black);
		m_portArea->SetCharacterFilter([](Nz::UInt32 character) 
		{
			if (character < U'0' || character > U'9')
				return false;

			return true;
		});

		Ndk::BoxLayout* buttonLayout = m_serverConfigLayout->Add<Ndk::BoxLayout>(Ndk::BoxLayoutOrientation_Horizontal);

		m_backButton = buttonLayout->Add<Ndk::ButtonWidget>();
		m_backButton->UpdateText(Nz::SimpleTextDrawer::Draw("Back", 24));

		m_backButton->OnButtonTrigger.Connect([this](const Ndk::ButtonWidget*)
		{
			OnBackPressed();
		});

		m_startServerButton = buttonLayout->Add<Ndk::ButtonWidget>();
		m_startServerButton->UpdateText(Nz::SimpleTextDrawer::Draw("Start server", 24));
		
		m_startServerButton->OnButtonTrigger.Connect([this](const Ndk::ButtonWidget*)
		{
			OnStartServerPressed();
		});
	}

	void StartServerState::Enter(Ndk::StateMachine& fsm)
	{
		AbstractState::Enter(fsm);

		const ConfigFile& playerConfig = GetStateData().app->GetPlayerSettings();

		m_gamemodeArea->SetText(playerConfig.GetStringValue("StartServer.Gamemode"));
		m_mapArea->SetText(playerConfig.GetStringValue("StartServer.Map"));
		m_portArea->SetText(std::to_string(playerConfig.GetIntegerValue<Nz::UInt16>("StartServer.Port")));
	}

	bool StartServerState::Update(Ndk::StateMachine& fsm, float elapsedTime)
	{
		if (!AbstractState::Update(fsm, elapsedTime))
			return false;

		if (m_nextState)
			fsm.ChangeState(std::move(m_nextState));

		return true;
	}

	void StartServerState::OnBackPressed()
	{
		m_nextState = std::move(m_previousState);
	}

	void StartServerState::OnStartServerPressed()
	{
		Nz::String gamemode = m_gamemodeArea->GetText();
		if (gamemode.IsEmpty())
		{
			UpdateStatus("Error: blank gamemode", Nz::Color::Red);
			return;
		}

		Nz::String map = m_mapArea->GetText();
		if (map.IsEmpty())
		{
			UpdateStatus("Error: blank map", Nz::Color::Red);
			return;
		}

		Nz::String serverPort = m_portArea->GetText();
		if (serverPort.IsEmpty())
		{
			UpdateStatus("Error: blank server port", Nz::Color::Red);
			return;
		}

		long long rawPort;
		if (!serverPort.ToInteger(&rawPort) || rawPort < 0 || rawPort > 0xFFFF)
		{
			UpdateStatus("Error: " + serverPort.ToStdString() + " is not a valid port", Nz::Color::Red);
			return;
		}

		ConfigFile& playerConfig = GetStateData().app->GetPlayerSettings();
		playerConfig.SetStringValue("StartServer.Gamemode", gamemode.ToStdString());
		playerConfig.SetStringValue("StartServer.Map", map.ToStdString());
		playerConfig.SetIntegerValue("StartServer.Port", rawPort);

		GetStateData().app->SavePlayerConfig();

		try
		{
			m_nextState = std::make_shared<ServerState>(GetStateDataPtr(), static_cast<Nz::UInt16>(rawPort), gamemode.ToStdString(), map.ToStdString(), shared_from_this());
		}
		catch (const std::exception& e)
		{
			UpdateStatus("Failed to start server: " + std::string(e.what()), Nz::Color::Red);
		}
	}

	void StartServerState::LayoutWidgets()
	{
		Nz::Vector2f canvasSize = GetStateData().canvas->GetSize();
		Nz::Vector2f center = canvasSize / 2.f;

		constexpr float padding = 10.f;

		std::array<Ndk::BaseWidget*, 2> widgets = {
			m_statusLabel,
			m_serverConfigLayout
		};

		float totalSize = padding * (widgets.size() - 1);
		for (Ndk::BaseWidget* widget : widgets)
			totalSize += widget->GetSize().y;

		Nz::Vector2f cursor = center;
		cursor.y -= totalSize / 2.f;

		m_serverConfigLayout->Resize(m_serverConfigLayout->GetSize()); //< Force layout

		m_statusLabel->SetPosition({ 0.f, cursor.y, 0.f });
		m_statusLabel->CenterHorizontal();
		cursor.y += m_statusLabel->GetSize().y + padding;

		m_serverConfigLayout->SetPosition({ 0.f, cursor.y, 0.f });
		m_serverConfigLayout->CenterHorizontal();
		cursor.y += m_serverConfigLayout->GetSize().y + padding;
	}

	void StartServerState::UpdateStatus(const std::string& status, const Nz::Color& color)
	{
		m_statusLabel->UpdateText(Nz::SimpleTextDrawer::Draw(status, 24, 0L, color));
		m_statusLabel->CenterHorizontal();
		m_statusLabel->Show(true);
	}
}
