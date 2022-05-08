// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Systems/PostFrameCallbackSystem.hpp>
#include <CoreLib/LogSystem/Logger.hpp>
#include <CoreLib/Components/ScriptComponent.hpp>
#include <ClientLib/ClientMatch.hpp>

namespace bw
{
	PostFrameCallbackSystem::PostFrameCallbackSystem()
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

		if (scriptComponent.HasCallbacks(ElementEvent::PostFrame))
			m_frameUpdateEntities.Insert(entity);
		else
			m_frameUpdateEntities.Remove(entity);
	}

	void PostFrameCallbackSystem::OnUpdate(float /*elapsedTime*/)
	{
		for (entt::entity entity : m_frameUpdateEntities)
		{
			auto& scriptComponent = entity->GetComponent<ScriptComponent>();

			scriptComponent.ExecuteCallback<ElementEvent::PostFrame>();
		}
	}

	Ndk::SystemIndex PostFrameCallbackSystem::systemIndex;
}
