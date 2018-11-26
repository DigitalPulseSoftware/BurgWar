// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/Components/ScriptComponent.hpp>

namespace bw
{
	ScriptComponent::ScriptComponent(std::shared_ptr<const ScriptedElement> element, std::shared_ptr<SharedScriptingContext> context, int tableRef) :
	m_element(std::move(element)),
	m_context(std::move(context)),
	m_tableRef(tableRef)
	{
	}

	ScriptComponent::~ScriptComponent()
	{
		m_context->GetLuaInstance().DestroyReference(m_tableRef);
	}

	Ndk::ComponentIndex ScriptComponent::componentIndex;
}

