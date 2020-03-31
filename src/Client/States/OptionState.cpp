// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/OptionState.hpp>
#include <Client/ClientApp.hpp>

namespace bw
{
	OptionState::OptionState(std::shared_ptr<StateData> stateData, std::shared_ptr<AbstractState> previousState) :
	AbstractState(std::move(stateData)),
	m_previousState(std::move(previousState))
	{
		ConfigFile& playerConfig = GetStateData().app->GetPlayerSettings();

		m_optionWidget = CreateWidget<OptionWidget>(playerConfig);
		m_optionWidget->EnableBackground(true);
		m_optionWidget->SetBackgroundColor(Nz::Color(0, 0, 0, 80));

		m_backButton = CreateWidget<Ndk::ButtonWidget>();
		m_backButton->UpdateText(Nz::SimpleTextDrawer::Draw("Back", 24));
		m_backButton->Resize(m_backButton->GetPreferredSize());
		
		m_backButton->OnButtonTrigger.Connect([this](const Ndk::ButtonWidget*)
		{
			OnBackPressed();
		});
	}

	void OptionState::Leave(Ndk::StateMachine& fsm)
	{
		AbstractState::Leave(fsm);

		GetStateData().app->SavePlayerConfig();
	}

	bool OptionState::Update(Ndk::StateMachine& fsm, float elapsedTime)
	{
		if (m_nextState)
			fsm.ChangeState(std::move(m_nextState));

		return true;
	}

	void OptionState::LayoutWidgets()
	{
		Nz::Vector2f canvasSize = GetStateData().canvas->GetSize();

		m_optionWidget->Resize(canvasSize * 0.75f);
		m_optionWidget->Center();

		m_backButton->SetPosition(canvasSize.x - 10.f - m_backButton->GetWidth(), canvasSize.y - 10.f - m_backButton->GetHeight());
	}
	
	void OptionState::OnBackPressed()
	{
		m_nextState = std::move(m_previousState);
		m_previousState.reset(); // Ensure defined state
	}
}
