// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/Components/ScriptComponent.hpp>
#include <iostream>

namespace bw
{
	ScriptComponent::ScriptComponent(std::shared_ptr<const ScriptedElement> element, std::shared_ptr<SharedScriptingContext> context, sol::table entityTable) :
	m_element(std::move(element)),
	m_context(std::move(context)),
	m_entityTable(std::move(entityTable))
	{
	}

	ScriptComponent::~ScriptComponent() = default;

	void ScriptComponent::ExecuteCallback(const std::string& callbackName, const std::function<void()>& argumentFunction)
	{
		sol::protected_function callback = m_entityTable[callbackName];
		if (callback)
		{
			auto result = callback(m_entityTable, 42);
			if (!result.valid())
				std::cerr << callbackName << " entity callback failed: " << result.abandon << std::endl;

			//TODO: Handle return
		}
	}

	Ndk::ComponentIndex ScriptComponent::componentIndex;
}

