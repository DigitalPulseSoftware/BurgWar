// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Components/ScriptComponent.hpp>

namespace bw
{
	template<typename... Args>
	std::optional<sol::object> ScriptComponent::ExecuteCallback(const std::string& callbackName, Args&&... args)
	{
		sol::protected_function callback = m_entityTable[callbackName];
		if (callback)
		{
			auto co = m_context->CreateCoroutine(callback);

			auto result = co(m_entityTable, std::forward<Args>(args)...);
			if (!result.valid())
			{
				sol::error err = result;
				std::cerr << callbackName << " entity callback failed: " << err.what() << std::endl;
				return std::nullopt;
			}

			return result;
		}
		else
			return sol::nil;
	}

	inline const std::shared_ptr<ScriptingContext>& ScriptComponent::GetContext()
	{
		return m_context;
	}

	inline const std::shared_ptr<const ScriptedElement>& ScriptComponent::GetElement() const
	{
		return m_element;
	}

	inline std::optional<std::reference_wrapper<const EntityProperty>> ScriptComponent::GetProperty(const std::string& keyName) const
	{
		// Check specific value
		if (auto it = m_properties.find(keyName); it != m_properties.end())
			return it->second;

		// Check default value
		if (auto it = m_element->properties.find(keyName); it != m_element->properties.end())
			return it->second.defaultValue;

		// Not found, return nil for now (should we throw an error?)
		return std::nullopt;
	}

	inline const EntityProperties& ScriptComponent::GetProperties() const
	{
		return m_properties;
	}

	inline sol::table& ScriptComponent::GetTable()
	{
		return m_entityTable;
	}

	inline void ScriptComponent::UpdateElement(std::shared_ptr<const ScriptedElement> element)
	{
		m_element = std::move(element);
	}
}
