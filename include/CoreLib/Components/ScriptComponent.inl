// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Components/ScriptComponent.hpp>
#include <CoreLib/Utils.hpp>

namespace bw
{
	template<typename... Args>
	std::optional<sol::object> ScriptComponent::ExecuteCallback(ScriptingEvent event, Args&&... args)
	{
		const auto& callbacks = m_eventCallbacks[UnderlyingCast(event)];
		for (const auto& callback : callbacks)
		{
			auto co = m_context->CreateCoroutine(callback);

			auto result = co(m_entityTable, std::forward<Args>(args)...);
			if (!result.valid())
			{
				sol::error err = result;
				bwLog(m_logger, LogLevel::Error, "{} callback failed: {}", ToString(event), err.what());
				continue;
			}

			//return result; //< FIXME: Accumulate results
		}

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

	inline const EntityLogger& ScriptComponent::GetLogger() const
	{
		return m_logger;
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

	inline bool ScriptComponent::HasCallbacks(ScriptingEvent event) const
	{
		auto& callbacks = m_eventCallbacks[UnderlyingCast(event)];
		return !callbacks.empty();
	}

	inline void ScriptComponent::RegisterCallback(ScriptingEvent event, sol::protected_function callback)
	{
		auto& callbacks = m_eventCallbacks[UnderlyingCast(event)];
		callbacks.emplace_back(std::move(callback));
	}

	inline void ScriptComponent::SetNextTick(float seconds)
	{
		m_timeBeforeTick = seconds;
	}

	inline void ScriptComponent::UpdateElement(std::shared_ptr<const ScriptedElement> element)
	{
		m_element = std::move(element);
	}

	inline bool ScriptComponent::CanTriggerTick(float elapsedTime)
	{
		m_timeBeforeTick -= elapsedTime;
		return m_timeBeforeTick < 0.f;
	}
}
