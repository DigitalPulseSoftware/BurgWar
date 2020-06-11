// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Systems/PostFrameCallbackSystem.hpp>
#include <CoreLib/LogSystem/Logger.hpp>
#include <CoreLib/Components/ScriptComponent.hpp>
#include <ClientLib/LocalMatch.hpp>

namespace bw
{
	PostFrameCallbackSystem::PostFrameCallbackSystem(LocalMatch& match) :
	m_match(match)
	{
		Requires<ScriptComponent>();
		SetMaximumUpdateRate(0);
		SetUpdateOrder(100);
	}

	void PostFrameCallbackSystem::OnEntityRemoved(Ndk::Entity* entity)
	{
		m_frameUpdateEntities.Remove(entity);
	}

	void PostFrameCallbackSystem::OnEntityValidation(Ndk::Entity* entity, bool /*justAdded*/)
	{
		auto& scriptComponent = entity->GetComponent<ScriptComponent>();
		const auto& element = scriptComponent.GetElement();

		if (element->postFrameFunction)
			m_frameUpdateEntities.Insert(entity);
		else
			m_frameUpdateEntities.Remove(entity);
	}

	void PostFrameCallbackSystem::OnUpdate(float /*elapsedTime*/)
	{
		for (const Ndk::EntityHandle& entity : m_frameUpdateEntities)
		{
			auto& scriptComponent = entity->GetComponent<ScriptComponent>();
			const auto& element = scriptComponent.GetElement();

			assert(element->postFrameFunction);

			auto result = element->postFrameFunction(scriptComponent.GetTable());
			if (!result.valid())
			{
				sol::error err = result;
				bwLog(m_match.GetLogger(), LogLevel::Error, "OnPostFrame failed: {0}", err.what());
			}
		}
	}

	Ndk::SystemIndex PostFrameCallbackSystem::systemIndex;
}
