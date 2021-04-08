// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SCRIPTING_SHAREDGAMEMODE_HPP
#define BURGWAR_CORELIB_SCRIPTING_SHAREDGAMEMODE_HPP

#include <CoreLib/Export.hpp>
#include <CoreLib/Scripting/ScriptingContext.hpp>
#include <CoreLib/Scripting/GamemodeEventConnection.hpp>
#include <CoreLib/Scripting/GamemodeEvents.hpp>
#include <CoreLib/Scripting/ScriptedEvent.hpp>
#include <CoreLib/Scripting/ScriptedProperty.hpp>
#include <tsl/bhopscotch_map.h>
#include <array>
#include <string>

namespace bw
{
	struct GamemodeEventConnection;
	class SharedMatch;

	class BURGWAR_CORELIB_API SharedGamemode
	{
		public:
			SharedGamemode(SharedMatch& match, std::shared_ptr<ScriptingContext> scriptingContext, std::string gamemodeName, PropertyValueMap propertyValues);
			SharedGamemode(const SharedGamemode&) = delete;
			virtual ~SharedGamemode();

			template<GamemodeEvent Event, typename... Args>
			std::enable_if_t<!HasReturnValue(Event), bool> ExecuteCallback(Args... args);

			template<GamemodeEvent Event, typename... Args>
			std::enable_if_t<HasReturnValue(Event), std::optional<typename GamemodeEventData<Event>::ResultType>> ExecuteCallback(Args... args);

			template<typename... Args>
			std::optional<sol::object> ExecuteCustomCallback(std::size_t eventIndex, Args... args);

			inline const tsl::hopscotch_map<std::string /*key*/, ScriptedProperty>& GetProperties() const;
			inline const PropertyValueMap& GetPropertyValues() const;
			inline sol::table& GetTable();
			inline const sol::table& GetTable() const;

			inline bool HasCallbacks(GamemodeEvent event) const;

			inline void RegisterCallback(GamemodeEvent event, sol::main_protected_function callback, bool async);

			void Reload();

			SharedGamemode& operator=(const SharedGamemode&) = delete;

		protected:
			inline const std::string& GetGamemodeName() const;
			inline sol::table& GetGamemodeTable();
			inline std::optional<std::reference_wrapper<const PropertyValue>> GetProperty(const std::string& keyName) const;
			inline const std::shared_ptr<ScriptingContext>& GetScriptingContext() const;

			virtual void InitializeGamemode(const std::string& gamemodeName) = 0;

		private:
			sol::table LoadGamemode(const std::string& gamemodeName);
			void InitializeMetatable();
			GamemodeEventConnection RegisterCustomEvent(const sol::table& gamemodeTable, const std::string_view& event, sol::main_protected_function callback, bool async);
			GamemodeEventConnection RegisterEvent(const sol::table& gamemodeTable, const std::string_view& event, sol::main_protected_function callback, bool async);
			bool UnregisterEvent(const sol::table& gamemodeTable, const GamemodeEventConnection& connection);

			struct Callback
			{
				std::size_t callbackId;
				sol::main_protected_function callback;
				sol::table gamemodeTable;
				bool async = false;
			};

			std::array<std::vector<Callback>, GamemodeEventCount> m_eventCallbacks;
			std::shared_ptr<ScriptingContext> m_context;
			std::size_t m_nextCallbackId;
			std::string m_gamemodeName;
			std::vector<std::vector<Callback>> m_customEventCallbacks;
			std::vector<ScriptedEvent> m_customEvents;
			sol::table m_gamemodeTable;
			sol::table m_gamemodeMetatable;
			tsl::hopscotch_map<std::string /*key*/, ScriptedProperty> m_properties;
			tsl::hopscotch_map<std::string /*eventName*/, std::size_t> m_customEventByName;
			PropertyValueMap m_propertyValues;
			SharedMatch& m_sharedMatch;
	};
}

#include <CoreLib/Scripting/SharedGamemode.inl>

#endif
