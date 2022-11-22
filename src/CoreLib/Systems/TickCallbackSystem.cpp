// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Systems/TickCallbackSystem.hpp>
#include <CoreLib/SharedMatch.hpp>
#include <CoreLib/LogSystem/Logger.hpp>
#include <CoreLib/Components/ScriptComponent.hpp>
#include <cassert>

namespace bw
{
	TickCallbackSystem::TickCallbackSystem(entt::registry& registry, SharedMatch& match) :
	m_registry(registry),
	m_scriptObserver(registry, entt::collector.update<ScriptComponent>()),
	m_match(match)
	{
		m_scriptDestroyConnection = m_registry.on_destroy<ScriptComponent>().connect<&TickCallbackSystem::OnScriptDestroy>(this);
	}

	void TickCallbackSystem::OnScriptDestroy(entt::registry& registry, entt::entity entity)
	{
		assert(&m_registry == &registry);

		m_tickableEntities.erase(entity);
	}

	void TickCallbackSystem::Update(float elapsedTime)
	{
		for (entt::entity entity : m_scriptObserver)
		{
			auto& scriptComponent = m_registry.get<ScriptComponent>(entity);
			if (scriptComponent.HasCallbacks(ElementEvent::Tick))
				m_tickableEntities.emplace(entity);
			else
				m_tickableEntities.erase(entity);
		}

		for (entt::entity entity : m_tickableEntities)
		{
			auto& scriptComponent = m_registry.get<ScriptComponent>(entity);
			if (!scriptComponent.CanTriggerTick(elapsedTime)) //<FIXME: Due to reconciliation, this is not right
				continue;

			//FIXME
			scriptComponent.ExecuteCallback<ElementEvent::Tick>();

			/*auto result = element->tickFunction(scriptComponent.GetTable());
			if (result.valid())
			{
				sol::object retObj = result;
				if (retObj.is<float>())
					scriptComponent.SetNextTick(retObj.as<float>());
			}
			else
			{
				sol::error err = result;
				bwLog(m_match.GetLogger(), LogLevel::Error, "OnTick failed: {0}", err.what());
			}*/
		}
	}
}
