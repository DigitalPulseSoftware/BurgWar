// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/Game/StatusState.hpp>
#include <Nazara/TextRenderer/SimpleTextDrawer.hpp>

namespace bw
{
	StatusState::StatusState(std::shared_ptr<StateData> stateData) :
	AbstractState(std::move(stateData))
	{
		m_statusLabel = CreateWidget<Nz::LabelWidget>();
	}

	void StatusState::Enter(Nz::StateMachine& fsm)
	{
		AbstractState::Enter(fsm);

		StateData& stateData = GetStateData();

		//stateData.world->GetSystem<Ndk::RenderSystem>().SetDefaultBackground(Nz::ColorBackground::New(Nz::Color(100, 185, 191)));
	}

	void StatusState::Leave(Nz::StateMachine& fsm)
	{
		AbstractState::Leave(fsm);

		StateData& stateData = GetStateData();

		//stateData.world->GetSystem<Ndk::RenderSystem>().SetDefaultBackground(nullptr);
	}

	void StatusState::UpdateStatus(const std::string& status, const Nz::Color& color)
	{
		m_statusLabel->UpdateText(Nz::SimpleTextDrawer::Draw(status, 24, 0L, color));
		m_statusLabel->Resize(m_statusLabel->GetPreferredSize());
		m_statusLabel->Center();
	}
}
