// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Components/ScriptComponent.hpp>

namespace bw
{
	ScriptComponent::ScriptComponent(const Logger& logger, std::shared_ptr<const ScriptedElement> element, std::shared_ptr<ScriptingContext> context, sol::table entityTable, PropertyValueMap properties) :
	m_eventCallbacks(element->eventCallbacks),
	m_customEventCallbacks(element->customEventCallbacks),
	m_element(std::move(element)),
	m_context(std::move(context)),
	m_nextCallbackId(m_element->nextCallbackId),
	m_entityTable(std::move(entityTable)),
	m_logger(Ndk::EntityHandle::InvalidHandle, logger),
	m_properties(std::move(properties)),
	m_timeBeforeTick(0.f)
	{
	}

	ScriptComponent::~ScriptComponent() = default;

	void ScriptComponent::UpdateEntity(const Ndk::EntityHandle& entity)
	{
		m_entityTable["_Entity"] = entity;
		m_logger.UpdateEntity(entity);
	}

	void ScriptComponent::OnAttached()
	{
		UpdateEntity(m_entity);
	}

	Ndk::ComponentIndex ScriptComponent::componentIndex;
}

