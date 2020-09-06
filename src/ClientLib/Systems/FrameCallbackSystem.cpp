// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Systems/FrameCallbackSystem.hpp>
#include <CoreLib/LogSystem/Logger.hpp>
#include <CoreLib/Components/ScriptComponent.hpp>
#include <ClientLib/LocalMatch.hpp>

namespace bw
{
	FrameCallbackSystem::FrameCallbackSystem()
	{
		Requires<ScriptComponent>();
		SetMaximumUpdateRate(0);
		SetUpdateOrder(100);
	}

	void FrameCallbackSystem::OnEntityRemoved(Ndk::Entity* entity)
	{
		m_frameUpdateEntities.Remove(entity);
	}

	void FrameCallbackSystem::OnEntityValidation(Ndk::Entity* entity, bool /*justAdded*/)
	{
		auto& scriptComponent = entity->GetComponent<ScriptComponent>();

		if (scriptComponent.HasCallbacks(ElementEvent::Frame))
			m_frameUpdateEntities.Insert(entity);
		else
			m_frameUpdateEntities.Remove(entity);
	}

	void FrameCallbackSystem::OnUpdate(float /*elapsedTime*/)
	{
		for (const Ndk::EntityHandle& entity : m_frameUpdateEntities)
		{
			auto& scriptComponent = entity->GetComponent<ScriptComponent>();

			scriptComponent.ExecuteCallback<ElementEvent::Frame>();
		}
	}

	Ndk::SystemIndex FrameCallbackSystem::systemIndex;
}
