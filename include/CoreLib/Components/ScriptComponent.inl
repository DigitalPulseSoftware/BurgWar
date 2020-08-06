// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Components/ScriptComponent.hpp>
#include <CoreLib/Utils.hpp>

namespace bw
{
	template<ScriptingEvent Event, typename... Args>
	std::enable_if_t<!HasReturnValue(Event), bool> ScriptComponent::ExecuteCallback(const Args&... args)
	{
		using EventData = ScriptingEventData<Event>;

		const auto& callbacks = m_eventCallbacks[UnderlyingCast(Event)];
		if (callbacks.empty())
			return true;

		bool ret = false;

		for (const auto& callbackData : callbacks)
		{
			sol::protected_function_result callbackResult;
			if (callbackData.async)
			{
				auto co = m_context->CreateCoroutine(callbackData.callback);
				callbackResult = co(m_entityTable, args...);
			}
			else
				callbackResult = callbackData.callback(m_entityTable, args...);

			if (!callbackResult.valid())
			{
				sol::error err = callbackResult;
				bwLog(m_logger, LogLevel::Error, "{} callback failed: {}", ToString(Event), err.what());

				if constexpr (!EventData::FatalError)
					continue;

				return false;
			}

			ret = true;
		}

		return ret;
	}

	template<ScriptingEvent Event, typename... Args>
	std::enable_if_t<HasReturnValue(Event), std::optional<typename ScriptingEventData<Event>::ResultType>> ScriptComponent::ExecuteCallback(const Args&... args)
	{
		using EventData = ScriptingEventData<Event>;
		using ResultType = typename EventData::ResultType;

		std::optional<ResultType> combinedResult;

		const auto& callbacks = m_eventCallbacks[UnderlyingCast(Event)];
		for (const auto& callbackData : callbacks)
		{
			assert(!callbackData.async);

			auto callbackResult = callbackData.callback(m_entityTable, args...);
			if (!callbackResult.valid())
			{
				sol::error err = callbackResult;
				bwLog(m_logger, LogLevel::Error, "{} callback failed: {}", ToString(Event), err.what());

				if constexpr (!EventData::FatalError)
					continue;

				return {};
			}

			combinedResult = EventData::Combinator(combinedResult, callbackResult.get<ResultType>());
		}

		return combinedResult;
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

	inline void ScriptComponent::RegisterCallback(ScriptingEvent event, sol::protected_function callback, bool async)
	{
		auto& callbacks = m_eventCallbacks[UnderlyingCast(event)];
		auto& callbackData = callbacks.emplace_back();
		callbackData.async = async;
		callbackData.callback = std::move(callback);
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
