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

		m_background = CreateWidget<Ndk::BaseWidget>();
		m_background->EnableBackground(true);
		m_background->SetBackgroundColor(Nz::Color(0, 0, 0, 100));

		m_serverConfigLayout = m_background->Add<Ndk::BoxLayout>(Ndk::BoxLayoutOrientation_Vertical);
		m_serverConfigLayout->SetPosition(10.f, 10.f);

		m_title = m_serverConfigLayout->Add<Ndk::LabelWidget>();
		m_title->UpdateText(Nz::SimpleTextDrawer::Draw("Server configuration", 36));
		m_title->CenterHorizontal();

		Ndk::BoxLayout* gamemodeLayout = m_serverConfigLayout->Add<Ndk::BoxLayout>(Ndk::BoxLayoutOrientation_Horizontal);

		Ndk::LabelWidget* gamemodeLabel = gamemodeLayout->Add<Ndk::LabelWidget>();
		gamemodeLabel->UpdateText(Nz::SimpleTextDrawer::Draw("Gamemode: ", 24));

		m_gamemodeArea = gamemodeLayout->Add<Ndk::TextAreaWidget>();
		m_gamemodeArea->EnableBackground(true);
		m_gamemodeArea->SetBackgroundColor(Nz::Color::White);
		m_gamemodeArea->SetTextColor(Nz::Color::Black);
		m_gamemodeArea->SetMaximumHeight(m_gamemodeArea->GetMinimumHeight());

		Ndk::BoxLayout* mapLayout = m_serverConfigLayout->Add<Ndk::BoxLayout>(Ndk::BoxLayoutOrientation_Horizontal);

		Ndk::LabelWidget* mapLabel = mapLayout->Add<Ndk::LabelWidget>();
		mapLabel->UpdateText(Nz::SimpleTextDrawer::Draw("Map: ", 24));

		m_mapArea = mapLayout->Add<Ndk::TextAreaWidget>();
		m_mapArea->EnableBackground(true);
		m_mapArea->SetBackgroundColor(Nz::Color::White);
		m_mapArea->SetTextColor(Nz::Color::Black);
		m_mapArea->SetMaximumHeight(m_mapArea->GetMinimumHeight());

		Ndk::BoxLayout* portLayout = m_serverConfigLayout->Add<Ndk::BoxLayout>(Ndk::BoxLayoutOrientation_Horizontal);

		Ndk::LabelWidget* portLabel = portLayout->Add<Ndk::LabelWidget>();
		portLabel->UpdateText(Nz::SimpleTextDrawer::Draw("Port: ", 24));

		m_portArea = portLayout->Add<Ndk::TextAreaWidget>();
		m_portArea->EnableBackground(true);
		m_portArea->SetBackgroundColor(Nz::Color::White);
		m_portArea->SetTextColor(Nz::Color::Black);
		m_portArea->SetMaximumHeight(m_mapArea->GetMinimumHeight());
		m_portArea->SetCharacterFilter([](Nz::UInt32 character)
		{
			if (character < U'0' || character > U'9')
				return false;

			return true;
		});

		m_listServerCheckbox = m_serverConfigLayout->Add<Ndk::CheckboxWidget>();
		m_listServerCheckbox->UpdateText(Nz::SimpleTextDrawer::Draw("Register the server online", 24));

		m_nameLayout = m_serverConfigLayout->Add<Ndk::BoxLayout>(Ndk::BoxLayoutOrientation_Horizontal);

		Ndk::LabelWidget* nameLabel = m_nameLayout->Add<Ndk::LabelWidget>();
		nameLabel->UpdateText(Nz::SimpleTextDrawer::Draw("Name: ", 24));

		m_nameArea = m_nameLayout->Add<Ndk::TextAreaWidget>();
		m_nameArea->EnableBackground(true);
		m_nameArea->SetBackgroundColor(Nz::Color::White);
		m_nameArea->SetTextColor(Nz::Color::Black);
		m_nameArea->SetMaximumHeight(m_mapArea->GetMinimumHeight());

		m_descriptionLayout = m_serverConfigLayout->Add<Ndk::BoxLayout>(Ndk::BoxLayoutOrientation_Horizontal);

		Ndk::LabelWidget* descLabel = m_descriptionLayout->Add<Ndk::LabelWidget>();
		descLabel->UpdateText(Nz::SimpleTextDrawer::Draw("Description: ", 24));

		m_descriptionArea = m_descriptionLayout->Add<Ndk::TextAreaWidget>();
		m_descriptionArea->EnableBackground(true);
		m_descriptionArea->SetBackgroundColor(Nz::Color::White);
		m_descriptionArea->SetTextColor(Nz::Color::Black);
		m_descriptionArea->SetMaximumHeight(m_mapArea->GetMinimumHeight());

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

		m_descriptionArea->SetText(playerConfig.GetStringValue("StartServer.Description"));
		m_gamemodeArea->SetText(playerConfig.GetStringValue("StartServer.Gamemode"));
		m_listServerCheckbox->SetState((playerConfig.GetBoolValue("StartServer.ListServer")) ? Ndk::CheckboxState_Checked : Ndk::CheckboxState_Unchecked);
		m_mapArea->SetText(playerConfig.GetStringValue("StartServer.Map"));
		m_nameArea->SetText(playerConfig.GetStringValue("StartServer.Name"));
		m_portArea->SetText(std::to_string(playerConfig.GetIntegerValue<Nz::UInt16>("StartServer.Port")));
	}

	bool StartServerState::Update(Ndk::StateMachine& fsm, float elapsedTime)
	{
		if (!AbstractState::Update(fsm, elapsedTime))
			return false;

		if (m_nextGameState)
			fsm.ResetState(std::move(m_nextGameState));
		else if (m_nextState)
			fsm.ChangeState(std::move(m_nextState));

		return true;
	}

	void StartServerState::OnBackPressed()
	{
		m_nextState = std::move(m_previousState);
	}

	void StartServerState::OnStartServerPressed()
	{
		ClientApp* app = GetStateData().app;

		std::string gamemode = m_gamemodeArea->GetText().ToStdString();
		if (gamemode.empty())
		{
			UpdateStatus("Error: blank gamemode", Nz::Color::Red);
			return;
		}

		std::string map = m_mapArea->GetText().ToStdString();
		if (map.empty())
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

		ConfigFile& playerConfig = app->GetPlayerSettings();

		bool listServer = (m_listServerCheckbox->GetState() == Ndk::CheckboxState_Checked);
		std::string serverName;
		std::string serverDesc;
		if (listServer)
		{
			serverName = m_nameArea->GetText().ToStdString();
			if (serverName.empty())
			{
				UpdateStatus("Error: blank server name", Nz::Color::Red);
				return;
			}

			if (serverName.size() > 32)
			{
				UpdateStatus("Error: server name is too long", Nz::Color::Red);
				return;
			}

			serverDesc = m_descriptionArea->GetText().ToStdString();
			if (serverDesc.size() > 1024)
			{
				UpdateStatus("Error: server description is too long", Nz::Color::Red);
				return;
			}

			playerConfig.SetStringValue("StartServer.Description", serverDesc);
			playerConfig.SetStringValue("StartServer.Name", serverName);
		}

		playerConfig.SetBoolValue("StartServer.ListServer", listServer);

		playerConfig.SetStringValue("StartServer.Gamemode", gamemode);
		playerConfig.SetStringValue("StartServer.Map", map);
		playerConfig.SetIntegerValue("StartServer.Port", rawPort);

		app->SavePlayerConfig();

		try
		{
			const ConfigFile& config = app->GetConfig();

			Match::GamemodeSettings gamemodeSettings;
			gamemodeSettings.name = std::move(gamemode);

			Match::MatchSettings matchSettings;
			matchSettings.maxPlayerCount = 16;
			matchSettings.name = std::move(serverName);
			matchSettings.registerToMasterServer = listServer;
			matchSettings.port = static_cast<Nz::UInt16>(rawPort);
			matchSettings.tickDuration = 1.f / config.GetFloatValue<float>("ServerSettings.TickRate");

			Match::ModSettings modSettings;

			// FIXME: Allow to select enabled mods
			for (auto&& [modId, mod] : app->GetMods())
				modSettings.enabledMods[modId] = Match::ModSettings::ModEntry{};

			// Load map
			if (!EndsWith(map, ".bmap"))
			{
				if (std::filesystem::is_directory(map))
					matchSettings.map = Map::LoadFromDirectory(map);
				else
					matchSettings.map = Map::LoadFromBinary(map + ".bmap");
			}
			else
				matchSettings.map = Map::LoadFromBinary(map);

			m_nextGameState = std::make_shared<ServerState>(GetStateDataPtr(), std::move(matchSettings), std::move(gamemodeSettings), std::move(modSettings), shared_from_this());
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

		std::array<Ndk::BaseWidget*, 8> widgets = {
			m_title,
			m_gamemodeArea,
			m_mapArea,
			m_portArea,
			m_listServerCheckbox,
			m_nameArea,
			m_descriptionArea,
			m_backButton
		};

		float totalSize = padding * (widgets.size() - 1);
		for (Ndk::BaseWidget* widget : widgets)
		{
			if (widget)
				totalSize += widget->GetSize().y;
		}

		m_background->Resize(Nz::Vector2f(canvasSize.x / 2.f, totalSize));
		m_background->Center();
		m_serverConfigLayout->Resize(m_background->GetSize() - Nz::Vector2f(20.f, 20.f));

		m_statusLabel->SetPosition({ 0.f, m_background->GetPosition().y - m_statusLabel->GetHeight(), 0.f });
		m_statusLabel->CenterHorizontal();
	}

	void StartServerState::UpdateStatus(const std::string& status, const Nz::Color& color)
	{
		m_statusLabel->UpdateText(Nz::SimpleTextDrawer::Draw(status, 24, 0L, color));
		m_statusLabel->CenterHorizontal();
		m_statusLabel->Show(true);
	}
}
