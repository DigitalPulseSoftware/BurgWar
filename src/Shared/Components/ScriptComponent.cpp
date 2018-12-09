// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/Components/ScriptComponent.hpp>
#include <iostream>

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

	void ScriptComponent::ExecuteCallback(const std::string& callbackName, const std::function<int(Nz::LuaState&)>& argumentFunction)
	{
		Nz::LuaState& state = m_context->GetLuaInstance();

		state.PushReference(m_tableRef);

		Nz::CallOnExit popOnExit([&] { state.Pop(); });
		Nz::LuaType initType = state.GetField(callbackName);

		if (initType != Nz::LuaType_Nil)
		{
			if (initType != Nz::LuaType_Function)
				throw std::runtime_error(callbackName + " must be a function if defined");

			state.PushValue(-2);

			int paramCount = 1; // GM table itself
			if (argumentFunction)
				paramCount += argumentFunction(state);

			if (!state.Call(paramCount))
				std::cerr << callbackName << " gamemode callback failed: " << state.GetLastError() << std::endl;

			//TODO: Handle return
		}
	}

	Ndk::ComponentIndex ScriptComponent::componentIndex;
}

