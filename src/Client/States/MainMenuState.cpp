// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/MainMenuState.hpp>
#include <CoreLib/ConfigFile.hpp>
#include <Client/ClientApp.hpp>
#include <Client/States/JoinServerState.hpp>
#include <Client/States/OptionState.hpp>
#include <Client/States/StartServerState.hpp>
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
	MainMenuState::MainMenuState(std::shared_ptr<StateData> stateData) :
	AbstractState(std::move(stateData))
	{
		m_startServerButton = CreateWidget<Ndk::ButtonWidget>();
		m_startServerButton->UpdateText(Nz::SimpleTextDrawer::Draw("Start server...", 24));
		m_startServerButton->Resize(m_startServerButton->GetPreferredSize());
		
		m_startServerButton->OnButtonTrigger.Connect([this](const Ndk::ButtonWidget*)
		{
			OnStartServerPressed();
		});

		m_joinServerButton = CreateWidget<Ndk::ButtonWidget>();
		m_joinServerButton->UpdateText(Nz::SimpleTextDrawer::Draw("Join server...", 24));
		m_joinServerButton->Resize(m_joinServerButton->GetPreferredSize());

		m_joinServerButton->OnButtonTrigger.Connect([this](const Ndk::ButtonWidget*)
		{
			OnJoinServerPressed();
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
	}

	bool MainMenuState::Update(Ndk::StateMachine& fsm, float elapsedTime)
	{
		if (!AbstractState::Update(fsm, elapsedTime))
			return false;

		if (m_nextState)
			fsm.ChangeState(std::move(m_nextState));

		return true;
	}

	void MainMenuState::OnJoinServerPressed()
	{
		m_nextState = std::make_shared<JoinServerState>(GetStateDataPtr(), shared_from_this());
	}

	void MainMenuState::OnOptionPressed()
	{
		m_nextState = std::make_shared<OptionState>(GetStateDataPtr(), shared_from_this());
	}

	void MainMenuState::OnStartServerPressed()
	{
		m_nextState = std::make_shared<StartServerState>(GetStateDataPtr(), shared_from_this());
	}

	void MainMenuState::OnQuitPressed()
	{
		GetStateData().app->Quit();
	}

	void MainMenuState::LayoutWidgets()
	{
		Nz::Vector2f canvasSize = GetStateData().canvas->GetSize();
		Nz::Vector2f center = canvasSize / 2.f;

		constexpr float padding = 10.f;

		std::array<Ndk::BaseWidget*, 4> widgets = {
			m_startServerButton,
			m_joinServerButton,
			m_optionButton,
			m_quitButton
		};

		float maxWidth = 0.f;
		float totalSize = padding * (widgets.size() - 1);
		for (Ndk::BaseWidget* widget : widgets)
		{
			Nz::Vector2f size = widget->GetSize();

			maxWidth = std::max(maxWidth, size.x);
			totalSize += size.y;
		}

		Nz::Vector2f cursor = center;
		cursor.y -= totalSize / 2.f;

		for (Ndk::BaseWidget* widget : widgets)
		{
			widget->Resize({ maxWidth, widget->GetHeight() });
			widget->SetPosition({ 0.f, cursor.y, 0.f });
			widget->CenterHorizontal();
			cursor.y += widget->GetSize().y + padding;
		}
	}
}
