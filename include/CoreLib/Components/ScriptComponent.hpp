// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_COMPONENTS_SCRIPTCOMPONENT_HPP
#define BURGWAR_CORELIB_COMPONENTS_SCRIPTCOMPONENT_HPP

#include <CoreLib/PropertyValues.hpp>
#include <CoreLib/LogSystem/EntityLogger.hpp>
#include <CoreLib/Scripting/ElementEvents.hpp>
#include <CoreLib/Scripting/ScriptedElement.hpp>
#include <CoreLib/Scripting/ScriptingContext.hpp>
#include <NDK/Component.hpp>
#include <array>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace bw
{
	class ScriptComponent : public Ndk::Component<ScriptComponent>
	{
		friend class TickCallbackSystem;

		public:
			ScriptComponent(const Logger& logger, std::shared_ptr<const ScriptedElement> element, std::shared_ptr<ScriptingContext> context, sol::table entityTable, PropertyValueMap properties);
			~ScriptComponent();

			template<ElementEvent Event, typename... Args>
			std::enable_if_t<!HasReturnValue(Event), bool> ExecuteCallback(const Args&... args);

			template<ElementEvent Event, typename... Args>
			std::enable_if_t<HasReturnValue(Event), std::optional<typename ElementEventData<Event>::ResultType>> ExecuteCallback(const Args&... args);

			template<typename... Args>
			std::optional<sol::object> ExecuteCustomCallback(std::size_t eventIndex, const Args&... args);

			inline const std::shared_ptr<ScriptingContext>& GetContext();
			inline const std::shared_ptr<const ScriptedElement>& GetElement() const;
			inline const EntityLogger& GetLogger() const;
			inline std::optional<std::reference_wrapper<const PropertyValue>> GetProperty(const std::string& keyName) const;
			inline const PropertyValueMap& GetProperties() const;
			inline sol::table& GetTable();

			inline bool HasCallbacks(ElementEvent event) const;

			inline void RegisterCallback(ElementEvent event, sol::main_protected_function callback, bool async);
			inline void RegisterCallbackCustom(std::size_t eventIndex, sol::main_protected_function callback, bool async);

			inline void SetNextTick(float seconds);

			inline void UpdateElement(std::shared_ptr<const ScriptedElement> element);
			void UpdateEntity(const Ndk::EntityHandle& entity);

			static Ndk::ComponentIndex componentIndex;

		private:
			inline bool CanTriggerTick(float elapsedTime);
			void OnAttached() override;

			std::array<std::vector<ScriptedElement::Callback>, ElementEventCount> m_eventCallbacks;
			std::vector<std::vector<ScriptedElement::Callback>> m_customEventCallbacks;
			std::shared_ptr<const ScriptedElement> m_element;
			std::shared_ptr<ScriptingContext> m_context;
			sol::table m_entityTable;
			EntityLogger m_logger;
			PropertyValueMap m_properties;
			float m_timeBeforeTick;
	};
}

#include <CoreLib/Components/ScriptComponent.inl>

#endif
