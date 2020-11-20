// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/AbstractState.hpp>

namespace bw
{
	AbstractState::AbstractState(std::shared_ptr<StateData> stateData) :
	m_stateData(std::move(stateData)),
	m_isVisible(false)
	{
		m_onTargetChangeSizeSlot.Connect(m_stateData->window->OnRenderTargetSizeChange, [this](const Nz::RenderTarget*)
		{
			if (m_isVisible)
				LayoutWidgets(); 
		});
	}

	AbstractState::~AbstractState()
	{
		for (const auto& cleanupFunc : m_cleanupFunctions)
			cleanupFunc();

		for (WidgetEntry& entry : m_widgets)
			entry.widget->Destroy();
	}

	void AbstractState::Enter(Ndk::StateMachine& /*fsm*/)
	{
		m_isVisible = true;

		for (WidgetEntry& entry : m_widgets)
		{
			if (entry.wasVisible)
				entry.widget->Show();
		}

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

		LayoutWidgets();
	}

	void AbstractState::Leave(Ndk::StateMachine& /*fsm*/)
	{
		m_isVisible = false;

		for (WidgetEntry& entry : m_widgets)
		{
			entry.wasVisible = entry.widget->IsVisible();
			entry.widget->Hide();
		}

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
