// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/Game/ConnectionLostState.hpp>
#include <Client/States/MainMenuState.hpp>
#include <Nazara/Core/StateMachine.hpp>
#include <Nazara/TextRenderer/SimpleTextDrawer.hpp>
#include <Nazara/Widgets/LabelWidget.hpp>

namespace bw
{
	ConnectionLostState::ConnectionLostState(std::shared_ptr<StateData> stateData) :
	AbstractState(std::move(stateData)),
	m_timer(Nz::Time::Seconds(5))
	{
		Nz::LabelWidget* labelWidget = CreateWidget<Nz::LabelWidget>();
		labelWidget->UpdateText(Nz::SimpleTextDrawer::Draw("Connection lost.", 36, Nz::TextStyle_Regular, Nz::Color::Red()));
		labelWidget->Center();
	}

	bool ConnectionLostState::Update(Nz::StateMachine& fsm, Nz::Time elapsedTime)
	{
		if (!AbstractState::Update(fsm, elapsedTime))
			return false;

		if ((m_timer -= elapsedTime) < Nz::Time::Zero())
			fsm.ChangeState(std::make_shared<MainMenuState>(GetStateDataPtr()));

		return true;
	}
}
