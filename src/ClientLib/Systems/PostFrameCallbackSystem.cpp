// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Systems/PostFrameCallbackSystem.hpp>
#include <CoreLib/Components/ScriptComponent.hpp>
#include <ClientLib/ClientMatch.hpp>

namespace bw
{
	PostFrameCallbackSystem::PostFrameCallbackSystem(entt::registry& registry) :
	m_registry(registry)
	{
		m_scriptDestroyConnection = m_registry.on_destroy<ScriptComponent>().connect<&PostFrameCallbackSystem::OnScriptDestroy>(this);
	}

	void PostFrameCallbackSystem::Update(float /*elapsedTime*/)
	{
		for (entt::entity entity : m_frameUpdateEntities)
		{
			auto& scriptComponent = m_registry.get<ScriptComponent>(entity);

			scriptComponent.ExecuteCallback<ElementEvent::Frame>();
		}
	}

	void PostFrameCallbackSystem::OnScriptDestroy(entt::registry& registry, entt::entity entity)
	{
		assert(&m_registry == &registry);

		m_frameUpdateEntities.erase(entity);
	}
}
