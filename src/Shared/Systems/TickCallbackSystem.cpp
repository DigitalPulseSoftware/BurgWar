// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/Systems/TickCallbackSystem.hpp>
#include <Shared/Components/ScriptComponent.hpp>
#include <iostream>

namespace bw
{
	TickCallbackSystem::TickCallbackSystem()
	{
		Requires<ScriptComponent>();
		SetMaximumUpdateRate(0);
	}

	void TickCallbackSystem::OnEntityRemoved(Ndk::Entity* entity)
	{
		m_tickableEntities.Remove(entity);
	}

	void TickCallbackSystem::OnEntityValidation(Ndk::Entity* entity, bool justAdded)
	{
		auto& scriptComponent = entity->GetComponent<ScriptComponent>();
		const auto& element = scriptComponent.GetElement();

		if (element->tickFunction != -1)
			m_tickableEntities.Insert(entity);
		else
			m_tickableEntities.Remove(entity);
	}

	void TickCallbackSystem::OnUpdate(float elapsedTime)
	{
		for (const Ndk::EntityHandle& entity : m_tickableEntities)
		{
			auto& scriptComponent = entity->GetComponent<ScriptComponent>();

			const auto& element = scriptComponent.GetElement();
			Nz::LuaState& state = scriptComponent.GetContext()->GetLuaInstance();

			state.PushReference(element->tickFunction);
			state.PushReference(scriptComponent.GetTableRef());

			if (!state.Call(1))
				std::cerr << "OnTick failed: " << state.GetLastError() << std::endl;
		}
	}

	Ndk::SystemIndex TickCallbackSystem::systemIndex;
}
