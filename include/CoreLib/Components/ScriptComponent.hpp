// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_COMPONENTS_SCRIPTCOMPONENT_HPP
#define BURGWAR_CORELIB_COMPONENTS_SCRIPTCOMPONENT_HPP

#include <CoreLib/EntityProperties.hpp>
#include <CoreLib/Scripting/ScriptedElement.hpp>
#include <CoreLib/Scripting/ScriptingContext.hpp>
#include <NDK/Component.hpp>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace bw
{
	class ScriptComponent : public Ndk::Component<ScriptComponent>
	{
		public:
			ScriptComponent(std::shared_ptr<const ScriptedElement> element, std::shared_ptr<ScriptingContext> context, sol::table entityTable, EntityProperties properties);
			~ScriptComponent();

			template<typename... Args>
			std::optional<sol::object> ExecuteCallback(const std::string& callbackName, Args&&... args);

			inline const std::shared_ptr<ScriptingContext>& GetContext();
			inline const std::shared_ptr<const ScriptedElement>& GetElement() const;
			inline std::optional<std::reference_wrapper<const EntityProperty>> GetProperty(const std::string& keyName) const;
			inline const EntityProperties& GetProperties() const;
			inline sol::table& GetTable();

			inline void UpdateElement(std::shared_ptr<const ScriptedElement> element);

			static Ndk::ComponentIndex componentIndex;

		private:
			std::shared_ptr<const ScriptedElement> m_element;
			std::shared_ptr<ScriptingContext> m_context;
			sol::table m_entityTable;
			EntityProperties m_properties;
	};
}

#include <CoreLib/Components/ScriptComponent.inl>

#endif
