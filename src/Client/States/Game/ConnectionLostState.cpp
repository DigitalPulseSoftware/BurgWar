// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/Game/ConnectionLostState.hpp>
#include <Client/States/MainMenuState.hpp>
#include <Nazara/Utility/SimpleTextDrawer.hpp>
#include <NDK/StateMachine.hpp>
#include <Nazara/Widgets/LabelWidget.hpp>

namespace bw
{
	ConnectionLostState::ConnectionLostState(std::shared_ptr<StateData> stateData) :
	AbstractState(std::move(stateData)),
	m_timer(5.f)
	{
		Nz::LabelWidget* labelWidget = CreateWidget<Nz::LabelWidget>();
		labelWidget->UpdateText(Nz::SimpleTextDrawer::Draw("Connection lost.", 36, Nz::TextStyle_Regular, Nz::Color::Red));
		labelWidget->Center();
	}

	bool ConnectionLostState::Update(Ndk::StateMachine& fsm, float elapsedTime)
	{
		if (!AbstractState::Update(fsm, elapsedTime))
			return false;

		if ((m_timer -= elapsedTime) < 0.f)
			fsm.ChangeState(std::make_shared<MainMenuState>(GetStateDataPtr()));

		return true;
	}
}
