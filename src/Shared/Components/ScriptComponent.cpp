// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/Components/ScriptComponent.hpp>

namespace bw
{
	ScriptComponent::ScriptComponent(std::string className, std::shared_ptr<SharedScriptingContext> context, int tableRef) :
	m_context(std::move(context)),
	m_className(std::move(className)),
	m_tableRef(tableRef)
	{
	}

	ScriptComponent::~ScriptComponent()
	{
		m_context->GetLuaInstance().DestroyReference(m_tableRef);
	}

	Ndk::ComponentIndex ScriptComponent::componentIndex;
}

