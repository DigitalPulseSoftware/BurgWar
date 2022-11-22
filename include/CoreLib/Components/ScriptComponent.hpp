// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_COMPONENTS_SCRIPTCOMPONENT_HPP
#define BURGWAR_CORELIB_COMPONENTS_SCRIPTCOMPONENT_HPP

#include <CoreLib/Export.hpp>
#include <CoreLib/PropertyValues.hpp>
#include <CoreLib/LogSystem/EntityLogger.hpp>
#include <CoreLib/Scripting/ElementEvents.hpp>
#include <CoreLib/Scripting/ScriptedElement.hpp>
#include <CoreLib/Scripting/ScriptingContext.hpp>
#include <array>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace bw
{
	class BURGWAR_CORELIB_API ScriptComponent
	{
		friend class TickCallbackSystem;

		public:
			ScriptComponent(const Logger& logger, std::shared_ptr<const ScriptedElement> element, std::shared_ptr<ScriptingContext> context, sol::table entityTable, PropertyValueMap properties);
			ScriptComponent(const ScriptComponent&) = delete;
			ScriptComponent(ScriptComponent&&) noexcept = default;
			~ScriptComponent();

			template<ElementEvent Event, typename... Args>
			std::enable_if_t<!HasReturnValue(Event), bool> ExecuteCallback(Args... args);

			template<ElementEvent Event, typename... Args>
			std::enable_if_t<HasReturnValue(Event), std::optional<typename ElementEventData<Event>::ResultType>> ExecuteCallback(Args... args);

			template<typename... Args>
			std::optional<sol::object> ExecuteCustomCallback(std::size_t eventIndex, Args... args);

			inline const std::shared_ptr<ScriptingContext>& GetContext();
			inline const std::shared_ptr<const ScriptedElement>& GetElement() const;
			inline const EntityLogger& GetLogger() const;
			inline std::optional<std::reference_wrapper<const PropertyValue>> GetProperty(const std::string& keyName) const;
			inline const PropertyValueMap& GetProperties() const;
			inline sol::table& GetTable();

			inline bool HasCallbacks(ElementEvent event) const;

			inline std::size_t RegisterCallback(ElementEvent event, sol::main_protected_function callback, bool async);
			inline std::size_t RegisterCallbackCustom(std::size_t eventIndex, sol::main_protected_function callback, bool async);

			inline void SetNextTick(float seconds);

			inline bool UnregisterCallback(ElementEvent event, std::size_t callbackId);
			inline bool UnregisterCallbackCustom(std::size_t eventIndex, std::size_t callbackId);

			inline void UpdateElement(std::shared_ptr<const ScriptedElement> element);
			void UpdateEntity(entt::entity entity);

			ScriptComponent& operator=(const ScriptComponent&) = delete;
			ScriptComponent& operator=(ScriptComponent&&) noexcept = default;

			static constexpr auto in_place_delete = true;

		private:
			inline bool CanTriggerTick(float elapsedTime);

			std::array<std::vector<ScriptedElement::Callback>, ElementEventCount> m_eventCallbacks;
			std::vector<std::vector<ScriptedElement::Callback>> m_customEventCallbacks;
			std::shared_ptr<const ScriptedElement> m_element;
			std::shared_ptr<ScriptingContext> m_context;
			std::unique_ptr<EntityLogger> m_logger;
			std::size_t m_nextCallbackId;
			sol::table m_entityTable;
			PropertyValueMap m_properties;
			float m_timeBeforeTick;
	};
}

#include <CoreLib/Components/ScriptComponent.inl>

#endif
