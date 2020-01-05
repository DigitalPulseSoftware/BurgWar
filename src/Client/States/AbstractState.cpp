// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/AbstractState.hpp>

namespace bw
{
	AbstractState::AbstractState(std::shared_ptr<StateData> stateData) :
	m_stateData(std::move(stateData)),
	m_isVisible(false)
	{
	}

	AbstractState::~AbstractState()
	{
		for (const auto& cleanupFunc : m_cleanupFunctions)
			cleanupFunc();

		for (Ndk::BaseWidget* widget : m_widgets)
			widget->Destroy();
	}

	void AbstractState::Enter(Ndk::StateMachine& /*fsm*/)
	{
		m_isVisible = true;

		m_onTargetChangeSizeSlot.Connect(m_stateData->window->OnRenderTargetSizeChange, [this](const Nz::RenderTarget*) { LayoutWidgets(); });

		for (Ndk::BaseWidget* widget : m_widgets)
			widget->Show(true);

		for (auto it = m_entities.begin(); it != m_entities.end();)
		{
			const Ndk::EntityHandle& entity = *it;
			if (entity)
			{
				entity->Enable();
				++it;
			}
			else
				it = m_entities.erase(it);
		}
	}

	void AbstractState::Leave(Ndk::StateMachine& /*fsm*/)
	{
		m_isVisible = false;
		m_onTargetChangeSizeSlot.Disconnect();

		for (Ndk::BaseWidget* widget : m_widgets)
			widget->Show(false);

		for (auto it = m_entities.begin(); it != m_entities.end();)
		{
			const Ndk::EntityHandle& entity = *it;
			if (entity)
			{
				entity->Disable();
				++it;
			}
			else
				it = m_entities.erase(it);
		}
	}

	bool AbstractState::Update(Ndk::StateMachine& /*fsm*/, float /*elapsedTime*/)
	{
		return true;
	}

	void AbstractState::LayoutWidgets()
	{
	}
}
