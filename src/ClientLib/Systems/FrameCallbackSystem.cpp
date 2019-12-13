// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Systems/FrameCallbackSystem.hpp>
#include <CoreLib/LogSystem/Logger.hpp>
#include <CoreLib/Components/ScriptComponent.hpp>
#include <ClientLib/LocalMatch.hpp>

namespace bw
{
	FrameCallbackSystem::FrameCallbackSystem(LocalMatch& match) :
	m_match(match)
	{
		Requires<ScriptComponent>();
		SetMaximumUpdateRate(0);
		SetUpdateOrder(100);
	}

	void FrameCallbackSystem::OnEntityRemoved(Ndk::Entity* entity)
	{
		m_frameUpdateEntities.Remove(entity);
	}

	void FrameCallbackSystem::OnEntityValidation(Ndk::Entity* entity, bool justAdded)
	{
		auto& scriptComponent = entity->GetComponent<ScriptComponent>();
		const auto& element = scriptComponent.GetElement();

		if (element->frameFunction)
			m_frameUpdateEntities.Insert(entity);
		else
			m_frameUpdateEntities.Remove(entity);
	}

	void FrameCallbackSystem::OnUpdate(float elapsedTime)
	{
		for (const Ndk::EntityHandle& entity : m_frameUpdateEntities)
		{
			auto& scriptComponent = entity->GetComponent<ScriptComponent>();
			const auto& element = scriptComponent.GetElement();

			assert(element->frameFunction);

			auto result = element->frameFunction(scriptComponent.GetTable());
			if (!result.valid())
			{
				sol::error err = result;
				bwLog(m_match.GetLogger(), LogLevel::Error, "OnFrame failed: {0}", err.what());
			}
		}
	}

	Ndk::SystemIndex FrameCallbackSystem::systemIndex;
}
