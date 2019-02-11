// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <GameLibShared/Components/ScriptComponent.hpp>
#include <iostream>

namespace bw
{
	ScriptComponent::ScriptComponent(std::shared_ptr<const ScriptedElement> element, std::shared_ptr<SharedScriptingContext> context, sol::table entityTable, EntityProperties properties) :
	m_element(std::move(element)),
	m_context(std::move(context)),
	m_entityTable(std::move(entityTable)),
	m_properties(std::move(properties))
	{
	}

	ScriptComponent::~ScriptComponent() = default;

	Ndk::ComponentIndex ScriptComponent::componentIndex;
}

