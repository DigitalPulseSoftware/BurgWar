// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_COMPONENTS_SCRIPTCOMPONENT_HPP
#define BURGWAR_CORELIB_COMPONENTS_SCRIPTCOMPONENT_HPP

#include <CoreLib/EntityProperties.hpp>
#include <CoreLib/LogSystem/EntityLogger.hpp>
#include <CoreLib/Scripting/Events.hpp>
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
			ScriptComponent(const Logger& logger, std::shared_ptr<const ScriptedElement> element, std::shared_ptr<ScriptingContext> context, sol::table entityTable, EntityProperties properties);
			~ScriptComponent();

			template<typename... Args>
			std::optional<sol::object> ExecuteCallback(ScriptingEvent event, Args&&... args);

			inline const std::shared_ptr<ScriptingContext>& GetContext();
			inline const std::shared_ptr<const ScriptedElement>& GetElement() const;
			inline const EntityLogger& GetLogger() const;
			inline std::optional<std::reference_wrapper<const EntityProperty>> GetProperty(const std::string& keyName) const;
			inline const EntityProperties& GetProperties() const;
			inline sol::table& GetTable();

			inline bool HasCallbacks(ScriptingEvent event) const;

			inline void RegisterCallback(ScriptingEvent event, sol::protected_function callback);

			inline void SetNextTick(float seconds);

			inline void UpdateElement(std::shared_ptr<const ScriptedElement> element);
			void UpdateEntity(const Ndk::EntityHandle& entity);

			static Ndk::ComponentIndex componentIndex;

		private:
			inline bool CanTriggerTick(float elapsedTime);
			void OnAttached() override;

			std::array<std::vector<sol::protected_function>, ScriptingEventCount> m_eventCallbacks;
			std::shared_ptr<const ScriptedElement> m_element;
			std::shared_ptr<ScriptingContext> m_context;
			sol::table m_entityTable;
			EntityLogger m_logger;
			EntityProperties m_properties;
			float m_timeBeforeTick;
	};
}

#include <CoreLib/Components/ScriptComponent.inl>

#endif
