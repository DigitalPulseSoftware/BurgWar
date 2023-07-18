// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/AbstractState.hpp>
#include <cassert>

namespace bw
{
	template<typename T, typename ...Args>
	void AbstractState::ConnectSignal(T& signal, Args&&... args)
	{
		m_cleanupFunctions.emplace_back([connection = signal.Connect(std::forward<Args>(args)...)]() mutable { connection.Disconnect(); });
	}

	template<typename T, typename... Args>
	T* AbstractState::CreateWidget(Args&&... args)
	{
		T* widget = m_stateData->canvas->Add<T>(std::forward<Args>(args)...);

		auto& entry = m_widgets.emplace_back();
		entry.widget = widget;

		if (!m_isVisible)
			entry.widget->Hide();

		return widget;
	}

	inline entt::handle AbstractState::CreateEntity()
	{
		entt::handle entity = m_stateData->world->CreateEntity();
		if (!m_isVisible)
			entity.emplace<Nz::DisabledComponent>();

		m_entities.emplace_back(entity);

		return entity;
	}

	inline void AbstractState::DestroyWidget(Nz::BaseWidget* widget)
	{
		auto it = std::find_if(m_widgets.begin(), m_widgets.end(), [&](const WidgetEntry& widgetEntity) { return widgetEntity.widget == widget; });
		assert(it != m_widgets.end());

		m_widgets.erase(it);

		widget->Destroy();
	}

	inline StateData& AbstractState::GetStateData()
	{
		return *m_stateData;
	}

	inline const StateData& AbstractState::GetStateData() const
	{
		return *m_stateData;
	}

	inline const std::shared_ptr<StateData>& AbstractState::GetStateDataPtr()
	{
		return m_stateData;
	}
}
