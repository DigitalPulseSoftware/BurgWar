// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Systems/TickCallbackSystem.hpp>
#include <CoreLib/SharedMatch.hpp>
#include <CoreLib/LogSystem/Logger.hpp>
#include <CoreLib/Components/ScriptComponent.hpp>

namespace bw
{
	TickCallbackSystem::TickCallbackSystem(SharedMatch& match) :
	m_match(match)
	{
		Requires<ScriptComponent>();
		SetMaximumUpdateRate(0);
	}

	void TickCallbackSystem::OnEntityRemoved(Ndk::Entity* entity)
	{
		m_tickableEntities.Remove(entity);
	}

	void TickCallbackSystem::OnEntityValidation(Ndk::Entity* entity, bool /*justAdded*/)
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
			if (!scriptComponent.CanTriggerTick(elapsedTime)) //<FIXME: Due to reconciliation, this is not right
				continue;

			const auto& element = scriptComponent.GetElement();

			assert(element->tickFunction);

			auto result = element->tickFunction(scriptComponent.GetTable());
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
			}
		}
	}

	Ndk::SystemIndex TickCallbackSystem::systemIndex;
}
