// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Components/ScriptComponent.hpp>
#include <CoreLib/Utils.hpp>

namespace bw
{
	template<ElementEvent Event, typename... Args>
	std::enable_if_t<!HasReturnValue(Event), bool> ScriptComponent::ExecuteCallback(Args... args) //< FIXME: Not const because of a bug in sol
	{
		using EventData = ElementEventData<Event>;

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

	template<ElementEvent Event, typename... Args>
	std::enable_if_t<HasReturnValue(Event), std::optional<typename ElementEventData<Event>::ResultType>> ScriptComponent::ExecuteCallback(Args... args) //< FIXME: Not const because of a bug in sol
	{
		using EventData = ElementEventData<Event>;
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
				else
					return {};
			}

			auto retOpt = callbackResult.template get<std::optional<ResultType>>();
			if (retOpt)
				combinedResult = EventData::Combinator(combinedResult, *retOpt);
		}

		return combinedResult;
	}

	template<typename... Args>
	std::optional<sol::object> ScriptComponent::ExecuteCustomCallback(std::size_t eventIndex, Args... args) //< FIXME: Not const because of a bug in sol
	{
		if (eventIndex >= m_customEventCallbacks.size())
			return sol::nil;

		const auto& callbacks = m_customEventCallbacks[eventIndex];
		if (callbacks.empty())
			return sol::nil;

		assert(eventIndex < m_element->customEvents.size());
		const auto& eventData = m_element->customEvents[eventIndex];
		if (eventData.returnType.empty())
		{
			// No return
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
					bwLog(m_logger, LogLevel::Error, "{} callback failed: {}", eventData.name, err.what());

					continue;
				}

				ret = true;
			}

			if (ret)
				return sol::nil;
			else
				return {};
		}
		else
		{
			std::optional<sol::object> combinedResult;

			for (const auto& callbackData : callbacks)
			{
				assert(!callbackData.async);

				auto callbackResult = callbackData.callback(m_entityTable, args...);
				if (!callbackResult.valid())
				{
					sol::error err = callbackResult;
					bwLog(m_logger, LogLevel::Error, "{} callback failed: {}", eventData.name, err.what());

					continue;
				}

				if (eventData.combinator && combinedResult.has_value())
				{
					auto combinatorResult = eventData.combinator(combinedResult, callbackResult);
					if (!callbackResult.valid())
					{
						sol::error err = callbackResult;
						bwLog(m_logger, LogLevel::Error, "{} combinator failed: {}", eventData.name, err.what());

						continue;
					}

					combinedResult.emplace(combinatorResult);
				}
				else
					combinedResult.emplace(callbackResult);
			}

			return combinedResult;
		}
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
		assert(m_logger);
		return *m_logger;
	}

	inline std::optional<std::reference_wrapper<const PropertyValue>> ScriptComponent::GetProperty(const std::string& keyName) const
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

	inline const PropertyValueMap& ScriptComponent::GetProperties() const
	{
		return m_properties;
	}

	inline sol::table& ScriptComponent::GetTable()
	{
		return m_entityTable;
	}

	inline bool ScriptComponent::HasCallbacks(ElementEvent event) const
	{
		auto& callbacks = m_eventCallbacks[UnderlyingCast(event)];
		return !callbacks.empty();
	}

	inline std::size_t ScriptComponent::RegisterCallback(ElementEvent event, sol::main_protected_function callback, bool async)
	{
		auto& callbacks = m_eventCallbacks[UnderlyingCast(event)];
		auto& callbackData = callbacks.emplace_back();
		callbackData.async = async;
		callbackData.callback = std::move(callback);
		callbackData.callbackId = m_nextCallbackId++;

		return callbackData.callbackId;
	}

	inline std::size_t ScriptComponent::RegisterCallbackCustom(std::size_t eventIndex, sol::main_protected_function callback, bool async)
	{
		if (m_customEventCallbacks.size() <= eventIndex)
			m_customEventCallbacks.resize(eventIndex + 1);

		auto& callbackData = m_customEventCallbacks[eventIndex].emplace_back();
		callbackData.async = async;
		callbackData.callback = std::move(callback);
		callbackData.callbackId = m_nextCallbackId++;

		return callbackData.callbackId;
	}

	inline void ScriptComponent::SetNextTick(float seconds)
	{
		m_timeBeforeTick = seconds;
	}

	inline bool ScriptComponent::UnregisterCallback(ElementEvent event, std::size_t callbackId)
	{
		auto& callbacks = m_eventCallbacks[UnderlyingCast(event)];

		for (auto it = callbacks.begin(); it != callbacks.end(); ++it)
		{
			if (it->callbackId == callbackId)
			{
				callbacks.erase(it);
				return true;
			}
		}

		return false;
	}

	inline bool ScriptComponent::UnregisterCallbackCustom(std::size_t eventIndex, std::size_t callbackId)
	{
		if (m_customEventCallbacks.size() <= eventIndex)
			return false;

		auto& callbacks = m_customEventCallbacks[eventIndex];

		for (auto it = callbacks.begin(); it != callbacks.end(); ++it)
		{
			if (it->callbackId == callbackId)
			{
				callbacks.erase(it);
				return true;
			}
		}

		return false;
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
