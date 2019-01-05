// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/Components/ScriptComponent.hpp>

namespace bw
{
	template<typename... Args>
	void ScriptComponent::ExecuteCallback(const std::string& callbackName, Args&&... args)
	{
		sol::protected_function callback = m_entityTable[callbackName];
		if (callback)
		{
			auto result = callback(m_entityTable, std::forward<Args>(args)...);
			if (!result.valid())
			{
				sol::error err = result;
				std::cerr << callbackName << " entity callback failed: " << err.what() << std::endl;
			}

			//TODO: Handle return
		}
	}

	inline const std::shared_ptr<SharedScriptingContext>& ScriptComponent::GetContext()
	{
		return m_context;
	}

	inline const std::shared_ptr<const ScriptedElement>& ScriptComponent::GetElement() const
	{
		return m_element;
	}

	inline sol::table& ScriptComponent::GetTable()
	{
		return m_entityTable;
	}
}
