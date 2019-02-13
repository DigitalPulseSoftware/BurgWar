// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Systems/TickCallbackSystem.hpp>
#include <CoreLib/Components/ScriptComponent.hpp>
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

		if (element->tickFunction)
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
			sol::protected_function onTick = element->tickFunction;

			auto result = onTick(scriptComponent.GetTable());
			if (!result.valid())
			{
				sol::error err = result;
				std::cerr << "OnTick failed: " << err.what() << std::endl;
			}
		}
	}

	Ndk::SystemIndex TickCallbackSystem::systemIndex;
}
