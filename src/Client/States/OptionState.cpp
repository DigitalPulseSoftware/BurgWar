// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/OptionState.hpp>
#include <Client/ClientAppComponent.hpp>

namespace bw
{
	OptionState::OptionState(std::shared_ptr<StateData> stateData, std::shared_ptr<AbstractState> previousState) :
	AbstractState(std::move(stateData)),
	m_previousState(std::move(previousState))
	{
		ConfigFile& playerConfig = GetStateData().appComponent->GetPlayerSettings();

		m_optionWidget = CreateWidget<OptionWidget>(playerConfig);
		m_optionWidget->EnableBackground(true);
		m_optionWidget->SetBackgroundColor(Nz::Color(0, 0, 0, 80));

		m_optionWidget->OnBackButtonTriggered.Connect([this](OptionWidget*)
		{
			OnBackPressed();
		});
	}

	void OptionState::Leave(Nz::StateMachine& fsm)
	{
		AbstractState::Leave(fsm);

		GetStateData().appComponent->SavePlayerConfig();
	}

	bool OptionState::Update(Nz::StateMachine& fsm, Nz::Time /*elapsedTime*/)
	{
		if (m_nextState)
			fsm.ChangeState(std::move(m_nextState));

		return true;
	}

	void OptionState::LayoutWidgets()
	{
		Nz::Vector2f canvasSize = GetStateData().canvas->GetSize();

		m_optionWidget->Resize(canvasSize * 0.5f);
		m_optionWidget->Center();
	}
	
	void OptionState::OnBackPressed()
	{
		m_nextState = std::move(m_previousState);
		m_previousState.reset(); // Ensure defined state
	}
}
