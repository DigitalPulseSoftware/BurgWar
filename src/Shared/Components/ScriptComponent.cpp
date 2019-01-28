// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/Components/ScriptComponent.hpp>
#include <iostream>

namespace bw
{
	ScriptComponent::ScriptComponent(std::shared_ptr<const ScriptedElement> element, std::shared_ptr<SharedScriptingContext> context, sol::table entityTable, Properties properties) :
	m_element(std::move(element)),
	m_context(std::move(context)),
	m_entityTable(std::move(entityTable)),
	m_properties(std::move(properties))
	{
	}

	ScriptComponent::~ScriptComponent() = default;

	Ndk::ComponentIndex ScriptComponent::componentIndex;
}

