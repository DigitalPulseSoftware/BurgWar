// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/LoginState.hpp>
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
	LoginState::LoginState(std::shared_ptr<StateData> stateData) :
	AbstractState(std::move(stateData))
	{
		m_statusLabel = CreateWidget<Ndk::LabelWidget>();
		m_statusLabel->Hide();

		m_serverLabel = CreateWidget<Ndk::LabelWidget>();
		m_serverLabel->UpdateText(Nz::SimpleTextDrawer::Draw("Server: ", 24));

		m_serverAddressLayout = CreateWidget<Ndk::BoxLayout>(Ndk::BoxLayoutOrientation_Horizontal);

		m_serverAddressArea = m_serverAddressLayout->Add<Ndk::TextAreaWidget>();
		m_serverAddressArea->EnableBackground(true);
		m_serverAddressArea->SetBackgroundColor(Nz::Color::White);
		m_serverAddressArea->Resize({ 250.f, 36.f });
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
		
		m_optionButton = CreateWidget<Ndk::ButtonWidget>();
		m_optionButton->UpdateText(Nz::SimpleTextDrawer::Draw("Option", 24));
		m_optionButton->Resize(m_optionButton->GetPreferredSize());
		
		m_optionButton->OnButtonTrigger.Connect([this](const Ndk::ButtonWidget*)
		{
			OnOptionPressed();
		});

		m_quitButton = CreateWidget<Ndk::ButtonWidget>();
		m_quitButton->UpdateText(Nz::SimpleTextDrawer::Draw("Quit", 24));
		m_quitButton->Resize(m_quitButton->GetPreferredSize());
		
		m_quitButton->OnButtonTrigger.Connect([this](const Ndk::ButtonWidget*)
		{
			OnQuitPressed();
		});

		m_startServerButton = CreateWidget<Ndk::ButtonWidget>();
		m_startServerButton->UpdateText(Nz::SimpleTextDrawer::Draw("Start server", 24));
		m_startServerButton->Resize(m_startServerButton->GetPreferredSize());
		
		m_startServerButton->OnButtonTrigger.Connect([this](const Ndk::ButtonWidget*)
		{
			OnStartServerPressed();
		});
	}

	void LoginState::Enter(Ndk::StateMachine& fsm)
	{
		AbstractState::Enter(fsm);

		const ConfigFile& playerConfig = GetStateData().app->GetPlayerSettings();

		m_serverAddressArea->SetText(playerConfig.GetStringValue("Server.Address"));
		m_serverPortArea->SetText(std::to_string(playerConfig.GetIntegerValue<Nz::UInt16>("Server.Port")));
	}

	bool LoginState::Update(Ndk::StateMachine& fsm, float elapsedTime)
	{
		if (!AbstractState::Update(fsm, elapsedTime))
			return false;

		if (m_nextGameState)
			fsm.ResetState(std::move(m_nextGameState));
		else if (m_nextState)
			fsm.ChangeState(std::move(m_nextState));

		return true;
	}

	void LoginState::OnConnectionPressed()
	{
		Nz::String serverHostname = m_serverAddressArea->GetText();
		if (serverHostname.IsEmpty())
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
			UpdateStatus("Error: " + serverPort.ToStdString() + " is not a valid port", Nz::Color::Red);
			return;
		}

		Nz::ResolveError resolveError;
		std::vector<Nz::HostnameInfo> serverAddresses = Nz::IpAddress::ResolveHostname(Nz::NetProtocol_Any, serverHostname, Nz::String::Number(rawPort), &resolveError);
		if (serverAddresses.empty())
		{
			UpdateStatus(std::string("Failed to resolve server address: ") + Nz::ErrorToString(resolveError), Nz::Color::Red);
			return;
		}

		ConfigFile& playerConfig = GetStateData().app->GetPlayerSettings();
		playerConfig.SetStringValue("Server.Address", serverHostname.ToStdString());
		playerConfig.SetIntegerValue("Server.Port", rawPort);

		GetStateData().app->SavePlayerConfig();

		m_nextGameState = std::make_shared<ConnectionState>(GetStateDataPtr(), serverAddresses.front().address);
	}

	void LoginState::OnOptionPressed()
	{
		m_nextState = std::make_shared<OptionState>(GetStateDataPtr(), shared_from_this());
	}

	void LoginState::OnStartServerPressed()
	{
		Nz::String serverPort = m_serverPortArea->GetText();
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
		playerConfig.SetIntegerValue("Server.Port", rawPort);

		GetStateData().app->SavePlayerConfig();

		try
		{
			m_nextGameState = std::make_shared<ServerState>(GetStateDataPtr(), static_cast<Nz::UInt16>(rawPort));
		}
		catch (const std::exception& e)
		{
			UpdateStatus("Failed to start server: " + std::string(e.what()), Nz::Color::Red);
		}
	}

	void LoginState::OnQuitPressed()
	{
		GetStateData().app->Quit();
	}

	void LoginState::LayoutWidgets()
	{
		Nz::Vector2f canvasSize = GetStateData().canvas->GetSize();
		Nz::Vector2f center = canvasSize / 2.f;

		constexpr float padding = 10.f;

		std::array<Ndk::BaseWidget*, 3> widgets = {
			m_statusLabel,
			m_serverAddressLayout,
			m_connectionButton
		};

		float totalSize = padding * (widgets.size() - 1);
		for (Ndk::BaseWidget* widget : widgets)
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

		m_startServerButton->SetPosition({ 0.f, cursor.y, 0.f });
		m_startServerButton->CenterHorizontal();
		cursor.y += m_startServerButton->GetSize().y + padding;

		m_optionButton->SetPosition(10.f, canvasSize.y - m_optionButton->GetSize().y - 10.f);
		m_quitButton->SetPosition(canvasSize.x - m_quitButton->GetSize().x - 10.f, canvasSize.y - m_quitButton->GetSize().y - 10.f);
	}

	void LoginState::UpdateStatus(const std::string& status, const Nz::Color& color)
	{
		m_statusLabel->UpdateText(Nz::SimpleTextDrawer::Draw(status, 24, 0L, color));
		m_statusLabel->CenterHorizontal();
		m_statusLabel->Show(true);
	}
}
