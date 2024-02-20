// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/StartServerState.hpp>
#include <CoreLib/ConfigFile.hpp>
#include <Client/ClientAppComponent.hpp>
#include <Nazara/Core/StateMachine.hpp>
#include <Client/States/OptionState.hpp>
#include <Client/States/Game/ConnectionState.hpp>
#include <Client/States/Game/ServerState.hpp>
#include <Nazara/Network/Algorithm.hpp>
#include <Nazara/Network/IpAddress.hpp>
#include <Nazara/Renderer/DebugDrawer.hpp>
#include <Nazara/Renderer/Renderer.hpp>
#include <Nazara/TextRenderer/SimpleTextDrawer.hpp>
#include <Nazara/Widgets.hpp>
#include <cassert>
#include <chrono>

namespace bw
{
	StartServerState::StartServerState(std::shared_ptr<StateData> stateData, std::shared_ptr<AbstractState> previousState) :
	AbstractState(std::move(stateData)),
	m_previousState(std::move(previousState))
	{
		m_statusLabel = CreateWidget<Nz::LabelWidget>();
		m_statusLabel->Hide();

		m_background = CreateWidget<Nz::BaseWidget>();
		m_background->EnableBackground(true);
		m_background->SetBackgroundColor(Nz::Color(0, 0, 0, 100));

		m_serverConfigLayout = m_background->Add<Nz::BoxLayout>(Nz::BoxLayoutOrientation::TopToBottom);
		m_serverConfigLayout->SetPosition({ 10.f, 10.f });

		m_title = m_serverConfigLayout->Add<Nz::LabelWidget>();
		m_title->UpdateText(Nz::SimpleTextDrawer::Draw("Server configuration", 36));
		m_title->CenterHorizontal();

		Nz::BoxLayout* gamemodeLayout = m_serverConfigLayout->Add<Nz::BoxLayout>(Nz::BoxLayoutOrientation::LeftToRight);

		Nz::LabelWidget* gamemodeLabel = gamemodeLayout->Add<Nz::LabelWidget>();
		gamemodeLabel->UpdateText(Nz::SimpleTextDrawer::Draw("Gamemode: ", 24));

		m_gamemodeArea = gamemodeLayout->Add<Nz::TextAreaWidget>();
		m_gamemodeArea->EnableBackground(true);
		m_gamemodeArea->SetBackgroundColor(Nz::Color::White());
		m_gamemodeArea->SetTextColor(Nz::Color::Black());
		m_gamemodeArea->SetMaximumHeight(m_gamemodeArea->GetMinimumHeight());

		Nz::BoxLayout* mapLayout = m_serverConfigLayout->Add<Nz::BoxLayout>(Nz::BoxLayoutOrientation::LeftToRight);

		Nz::LabelWidget* mapLabel = mapLayout->Add<Nz::LabelWidget>();
		mapLabel->UpdateText(Nz::SimpleTextDrawer::Draw("Map: ", 24));

		m_mapArea = mapLayout->Add<Nz::TextAreaWidget>();
		m_mapArea->EnableBackground(true);
		m_mapArea->SetBackgroundColor(Nz::Color::White());
		m_mapArea->SetTextColor(Nz::Color::Black());
		m_mapArea->SetMaximumHeight(m_mapArea->GetMinimumHeight());

		Nz::BoxLayout* portLayout = m_serverConfigLayout->Add<Nz::BoxLayout>(Nz::BoxLayoutOrientation::LeftToRight);

		Nz::LabelWidget* portLabel = portLayout->Add<Nz::LabelWidget>();
		portLabel->UpdateText(Nz::SimpleTextDrawer::Draw("Port: ", 24));

		m_portArea = portLayout->Add<Nz::TextAreaWidget>();
		m_portArea->EnableBackground(true);
		m_portArea->SetBackgroundColor(Nz::Color::White());
		m_portArea->SetTextColor(Nz::Color::Black());
		m_portArea->SetMaximumHeight(m_mapArea->GetMinimumHeight());
		m_portArea->SetCharacterFilter([](Nz::UInt32 character)
		{
			if (character < U'0' || character > U'9')
				return false;

			return true;
		});

		Nz::BoxLayout* listServerLayout = m_serverConfigLayout->Add<Nz::BoxLayout>(Nz::BoxLayoutOrientation::LeftToRight);

		Nz::LabelWidget* listServerLabel = listServerLayout->Add<Nz::LabelWidget>();
		listServerLabel->UpdateText(Nz::SimpleTextDrawer::Draw("Register the server online", 24));

		m_listServerCheckbox = listServerLayout->Add<Nz::CheckboxWidget>();

		m_nameLayout = m_serverConfigLayout->Add<Nz::BoxLayout>(Nz::BoxLayoutOrientation::LeftToRight);

		Nz::LabelWidget* nameLabel = m_nameLayout->Add<Nz::LabelWidget>();
		nameLabel->UpdateText(Nz::SimpleTextDrawer::Draw("Name: ", 24));

		m_nameArea = m_nameLayout->Add<Nz::TextAreaWidget>();
		m_nameArea->EnableBackground(true);
		m_nameArea->SetBackgroundColor(Nz::Color::White());
		m_nameArea->SetTextColor(Nz::Color::Black());
		m_nameArea->SetMaximumHeight(m_mapArea->GetMinimumHeight());

		m_descriptionLayout = m_serverConfigLayout->Add<Nz::BoxLayout>(Nz::BoxLayoutOrientation::LeftToRight);

		Nz::LabelWidget* descLabel = m_descriptionLayout->Add<Nz::LabelWidget>();
		descLabel->UpdateText(Nz::SimpleTextDrawer::Draw("Description: ", 24));

		m_descriptionArea = m_descriptionLayout->Add<Nz::TextAreaWidget>();
		m_descriptionArea->EnableBackground(true);
		m_descriptionArea->SetBackgroundColor(Nz::Color::White());
		m_descriptionArea->SetTextColor(Nz::Color::Black());
		m_descriptionArea->SetMaximumHeight(m_mapArea->GetMinimumHeight());

		Nz::BoxLayout* buttonLayout = m_serverConfigLayout->Add<Nz::BoxLayout>(Nz::BoxLayoutOrientation::LeftToRight);

		m_backButton = buttonLayout->Add<Nz::ButtonWidget>();
		m_backButton->UpdateText(Nz::SimpleTextDrawer::Draw("Back", 24));

		m_backButton->OnButtonTrigger.Connect([this](const Nz::ButtonWidget*)
		{
			OnBackPressed();
		});

		m_startServerButton = buttonLayout->Add<Nz::ButtonWidget>();
		m_startServerButton->UpdateText(Nz::SimpleTextDrawer::Draw("Start server", 24));
		
		m_startServerButton->OnButtonTrigger.Connect([this](const Nz::ButtonWidget*)
		{
			OnStartServerPressed();
		});
	}

	void StartServerState::Enter(Nz::StateMachine& fsm)
	{
		AbstractState::Enter(fsm);

		const ConfigFile& playerConfig = GetStateData().appComponent->GetPlayerSettings();

		m_descriptionArea->SetText(playerConfig.GetStringValue("StartServer.Description"));
		m_gamemodeArea->SetText(playerConfig.GetStringValue("StartServer.Gamemode"));
		m_listServerCheckbox->SetState((playerConfig.GetBoolValue("StartServer.ListServer")) ? Nz::CheckboxState::Checked : Nz::CheckboxState::Unchecked);
		m_mapArea->SetText(playerConfig.GetStringValue("StartServer.Map"));
		m_nameArea->SetText(playerConfig.GetStringValue("StartServer.Name"));
		m_portArea->SetText(std::to_string(playerConfig.GetIntegerValue<Nz::UInt16>("StartServer.Port")));
	}

	bool StartServerState::Update(Nz::StateMachine& fsm, Nz::Time elapsedTime)
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
		ClientAppComponent* app = GetStateData().appComponent;

		std::string gamemode = m_gamemodeArea->GetText();
		if (gamemode.empty())
		{
			UpdateStatus("Error: blank gamemode", Nz::Color::Red());
			return;
		}

		std::string map = m_mapArea->GetText();
		if (map.empty())
		{
			UpdateStatus("Error: blank map", Nz::Color::Red());
			return;
		}

		std::string serverPort = m_portArea->GetText();
		if (serverPort.empty())
		{
			UpdateStatus("Error: blank server port", Nz::Color::Red());
			return;
		}

		bool ok;
		long long rawPort = Nz::StringToNumber(serverPort, 10, &ok);
		if (!ok || rawPort < 0 || rawPort > 0xFFFF)
		{
			UpdateStatus("Error: " + serverPort + " is not a valid port", Nz::Color::Red());
			return;
		}

		ConfigFile& playerConfig = app->GetPlayerSettings();

		bool listServer = (m_listServerCheckbox->GetState() == Nz::CheckboxState::Checked);
		std::string serverName;
		std::string serverDesc;
		if (listServer)
		{
			serverName = m_nameArea->GetText();
			if (serverName.empty())
			{
				UpdateStatus("Error: blank server name", Nz::Color::Red());
				return;
			}

			if (serverName.size() > 32)
			{
				UpdateStatus("Error: server name is too long", Nz::Color::Red());
				return;
			}

			serverDesc = m_descriptionArea->GetText();
			if (serverDesc.size() > 1024)
			{
				UpdateStatus("Error: server description is too long", Nz::Color::Red());
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
			matchSettings.tickDuration = Nz::Time::TickDuration(config.GetFloatValue<float>("ServerSettings.TickRate"));

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
			UpdateStatus("Failed to start server: " + std::string(e.what()), Nz::Color::Red());
		}
	}

	void StartServerState::LayoutWidgets()
	{
		Nz::Vector2f canvasSize = GetStateData().canvas->GetSize();
		Nz::Vector2f center = canvasSize / 2.f;

		constexpr float padding = 10.f;

		std::array<Nz::BaseWidget*, 8> widgets = {
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
		for (Nz::BaseWidget* widget : widgets)
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
