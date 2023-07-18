// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Systems/FrameCallbackSystem.hpp>
#include <CoreLib/Components/ScriptComponent.hpp>
#include <ClientLib/ClientMatch.hpp>

namespace bw
{
	FrameCallbackSystem::FrameCallbackSystem(entt::registry& registry) :
	m_registry(registry)
	{
		m_scriptDestroyConnection = m_registry.on_destroy<ScriptComponent>().connect<&FrameCallbackSystem::OnScriptDestroy>(this);
	}

	void FrameCallbackSystem::Update(Nz::Time /*elapsedTime*/)
	{
		for (entt::entity entity : m_frameUpdateEntities)
		{
			auto& scriptComponent = m_registry.get<ScriptComponent>(entity);

			scriptComponent.ExecuteCallback<ElementEvent::Frame>();
		}
	}

	void FrameCallbackSystem::OnScriptDestroy(entt::registry& registry, entt::entity entity)
	{
		assert(&m_registry == &registry);

		m_frameUpdateEntities.erase(entity);
	}
}
