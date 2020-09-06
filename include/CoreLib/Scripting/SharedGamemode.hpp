// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SCRIPTING_SHAREDGAMEMODE_HPP
#define BURGWAR_CORELIB_SCRIPTING_SHAREDGAMEMODE_HPP

#include <CoreLib/Scripting/ScriptingContext.hpp>
#include <CoreLib/Scripting/GamemodeEvents.hpp>
#include <CoreLib/Scripting/ScriptedProperty.hpp>
#include <Thirdparty/tsl/bhopscotch_map.h>
#include <array>
#include <string>

namespace bw
{
	class SharedMatch;

	class SharedGamemode
	{
		public:
			SharedGamemode(SharedMatch& match, std::shared_ptr<ScriptingContext> scriptingContext, std::string gamemodeName, PropertyValueMap propertyValues);
			SharedGamemode(const SharedGamemode&) = delete;
			virtual ~SharedGamemode();

			template<GamemodeEvent Event, typename... Args>
			std::enable_if_t<!HasReturnValue(Event), bool> ExecuteCallback(const Args&... args);

			template<GamemodeEvent Event, typename... Args>
			std::enable_if_t<HasReturnValue(Event), std::optional<typename GamemodeEventData<Event>::ResultType>> ExecuteCallback(const Args&... args);

			inline const tsl::hopscotch_map<std::string /*key*/, ScriptedProperty>& GetProperties() const;
			inline const PropertyValueMap& GetPropertyValues() const;
			inline sol::table& GetTable();

			inline bool HasCallbacks(GamemodeEvent event) const;

			inline void RegisterCallback(GamemodeEvent event, sol::protected_function callback, bool async);

			virtual void Reload() = 0;

			SharedGamemode& operator=(const SharedGamemode&) = delete;

		protected:
			inline const std::string& GetGamemodeName() const;
			inline sol::table& GetGamemodeTable();
			inline std::optional<std::reference_wrapper<const PropertyValue>> GetProperty(const std::string& keyName) const;
			inline const std::shared_ptr<ScriptingContext>& GetScriptingContext() const;

			virtual void InitializeGamemode() = 0;
	
		private:
			void RegisterEvent(const sol::table& gamemodeTable, const std::string_view& event, sol::protected_function callback, bool async);

			struct Callback
			{
				sol::protected_function callback;
				bool async = false;
			};

			std::array<std::vector<Callback>, GamemodeEventCount> m_eventCallbacks;
			std::shared_ptr<ScriptingContext> m_context;
			std::string m_gamemodeName;
			sol::table m_gamemodeTable;
			tsl::hopscotch_map<std::string /*key*/, ScriptedProperty> m_properties;
			PropertyValueMap m_propertyValues;
			SharedMatch& m_sharedMatch;
	};
}

#include <CoreLib/Scripting/SharedGamemode.inl>

#endif
