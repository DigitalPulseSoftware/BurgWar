// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/SharedGamemode.hpp>
#include <CoreLib/Components/HealthComponent.hpp>
#include <CoreLib/Scripting/GamemodeEventConnection.hpp>
#include <Nazara/Math/Vector2.hpp>
#include <CoreLib/Match.hpp>
#include <CoreLib/Terrain.hpp>
#include <cassert>

namespace bw
{
	SharedGamemode::SharedGamemode(SharedMatch& match, std::shared_ptr<ScriptingContext> scriptingContext, std::string gamemodeName, PropertyValueMap propertyValues) :
	m_context(std::move(scriptingContext)),
	m_nextCallbackId(1),
	m_gamemodeName(std::move(gamemodeName)),
	m_propertyValues(std::move(propertyValues)),
	m_sharedMatch(match)
	{
	}

	SharedGamemode::~SharedGamemode() = default;

	void SharedGamemode::Reload()
	{
		InitializeMetatable();
		m_gamemodeTable = LoadGamemode(m_gamemodeName);
	}

	sol::table SharedGamemode::LoadGamemode(const std::string& gamemodeName)
	{
		auto resultOpt = m_context->Load("gamemodes/" + gamemodeName + ".lua", false);
		if (!resultOpt)
			throw std::runtime_error("failed to load gamemode " + gamemodeName + " data");

		sol::table gamemodeTable = resultOpt->as<sol::table>();
		gamemodeTable["__index"] = gamemodeTable;

		std::string baseGamemode = gamemodeTable.get_or("Base", std::string{});
		if (!baseGamemode.empty())
		{
			sol::table parentGamemodeTable = LoadGamemode(baseGamemode);
			gamemodeTable["Base"] = parentGamemodeTable;
			gamemodeTable[sol::metatable_key] = parentGamemodeTable;
		}
		else
			gamemodeTable[sol::metatable_key] = m_gamemodeMetatable;

		sol::object events = gamemodeTable.raw_get<sol::object>("Events");
		if (events)
		{
			sol::table gamemodeEvents = events.as<sol::table>();

			for (const auto& kv : gamemodeEvents)
			{
				sol::table propertyTable = kv.second;

				try
				{
					std::size_t eventIndex = m_customEvents.size();
					ScriptedEvent event = InitEventFromLua(eventIndex, propertyTable);

					auto it = m_customEventByName.find(event.name);
					if (it == m_customEventByName.end())
					{
						m_customEventByName.emplace(event.name, eventIndex);
						m_customEvents.emplace_back(std::move(event));
					}
					else
						throw std::runtime_error("Custom event " + event.name + " already exists");
				}
				catch (const std::exception& e)
				{
					std::string_view eventName = propertyTable.get_or<std::string_view>("Name", "<No name set>");
					bwLog(m_sharedMatch.GetLogger(), LogLevel::Error, "Failed to load custom event {0} for gamemode {1}: {2}", eventName, m_gamemodeName, e.what());
				}
			}
		}

		sol::object properties = gamemodeTable.raw_get<sol::object>("Properties");
		if (properties)
		{
			sol::table gamemodeProperties = properties.as<sol::table>();

			for (const auto& kv : gamemodeProperties)
			{
				sol::table propertyTable = kv.second;

				std::string propertyName = propertyTable["Name"];

				try
				{
					std::size_t propertyIndex = m_properties.size();

					ScriptedProperty property = InitPropertyFromLua(propertyIndex, propertyTable);

					auto it = m_properties.find(propertyName);
					if (it == m_properties.end())
						m_properties.emplace(std::move(propertyName), std::move(property));
					else
						throw std::runtime_error("Property " + propertyName + " already exists");
				}
				catch (const std::exception& e)
				{
					bwLog(m_sharedMatch.GetLogger(), LogLevel::Error, "Failed to load property {0} for gamemode {1}: {2}", propertyName, m_gamemodeName, e.what());
				}
			}
		}

		// Initialize gamemode
		sol::state& state = m_context->GetLuaState();
		state["ScriptedGamemode"] = [&]()
		{
			return gamemodeTable;
		};

		InitializeGamemode(gamemodeName);

		// Initialize gamemode entities
		SharedEntityStore& entityStore = m_sharedMatch.GetEntityStore();
		entityStore.LoadDirectory("gamemodes/" + gamemodeName + "/entities");
		entityStore.Resolve();

		SharedWeaponStore& weaponStore = m_sharedMatch.GetWeaponStore();
		weaponStore.LoadDirectory("gamemodes/" + gamemodeName + "/weapons");
		weaponStore.Resolve();

		state["ScriptedGamemode"] = sol::nil;

		return gamemodeTable;
	}

	void SharedGamemode::InitializeMetatable()
	{
		sol::state& state = m_context->GetLuaState();
		m_gamemodeMetatable = state.create_table();
		m_gamemodeMetatable["__index"] = m_gamemodeMetatable;

		m_gamemodeMetatable["Disconnect"] = [&](const sol::table& gamemodeTable, const GamemodeEventConnection& connection)
		{
			return UnregisterEvent(gamemodeTable, connection);
		};

		m_gamemodeMetatable["On"] = [&](const sol::table& gamemodeTable, const std::string_view& event, sol::main_protected_function callback)
		{
			return RegisterEvent(gamemodeTable, event, std::move(callback), false);
		};

		m_gamemodeMetatable["OnAsync"] = [&](const sol::table& gamemodeTable, const std::string_view& event, sol::main_protected_function callback)
		{
			return RegisterEvent(gamemodeTable, event, std::move(callback), true);
		};

		m_gamemodeMetatable["Trigger"] = [&](const sol::table& /*gamemodeTable*/, const std::string_view& event, sol::variadic_args parameters)
		{
			std::string eventName = std::string(event);
			auto it = m_customEventByName.find(eventName);
			if (it == m_customEventByName.end())
				throw std::runtime_error("unknown event " + eventName);

			const auto& eventData = m_customEvents[it->second];

			return ExecuteCustomCallback(eventData.index, parameters);
		};

		m_gamemodeMetatable["GetProperty"] = [&](sol::this_state s, const sol::table& /*table*/, const std::string& propertyName) -> sol::object
		{
			auto propertyVal = GetProperty(propertyName);
			if (propertyVal.has_value())
			{
				sol::state_view lua(s);
				return TranslatePropertyToLua(&m_sharedMatch, lua, propertyVal.value());
			}
			else
				return sol::nil;
		};
	}

	GamemodeEventConnection SharedGamemode::RegisterCustomEvent(const sol::table& gamemodeTable, const std::string_view& event, sol::main_protected_function callback, bool async)
	{
		std::string eventName(event);
		auto it = m_customEventByName.find(eventName);
		if (it == m_customEventByName.end())
			throw std::runtime_error("unknown event " + eventName);

		std::size_t eventIndex = it->second;
		const auto& eventData = m_customEvents[eventIndex];

		if (async && !eventData.returnType.empty())
			throw std::runtime_error("events returning a value cannot be async");

		if (m_customEventCallbacks.size() <= eventIndex)
			m_customEventCallbacks.resize(eventIndex + 1);

		auto& callbackData = m_customEventCallbacks[eventIndex].emplace_back();
		callbackData.async = async;
		callbackData.callback = std::move(callback);
		callbackData.callbackId = m_nextCallbackId++;
		callbackData.gamemodeTable = gamemodeTable;

		return GamemodeEventConnection{ eventIndex, callbackData.callbackId };
	}

	GamemodeEventConnection SharedGamemode::RegisterEvent(const sol::table& gamemodeTable, const std::string_view& event, sol::main_protected_function callback, bool async)
	{
		std::optional<GamemodeEvent> gamemodeEventOpt = RetrieveGamemodeEvent(event);
		if (!gamemodeEventOpt)
			return RegisterCustomEvent(gamemodeTable, event, std::move(callback), async);

		GamemodeEvent scriptingEvent = gamemodeEventOpt.value();
		std::size_t eventIndex = static_cast<std::size_t>(scriptingEvent);

		if (async && HasReturnValue(scriptingEvent))
			throw std::runtime_error("events returning a value cannot be async");

		auto& callbackData = m_eventCallbacks[eventIndex].emplace_back();
		callbackData.async = async;
		callbackData.callback = std::move(callback);
		callbackData.callbackId = m_nextCallbackId++;
		callbackData.gamemodeTable = gamemodeTable;

		return GamemodeEventConnection{ scriptingEvent, callbackData.callbackId };
	}

	bool SharedGamemode::UnregisterEvent(const sol::table& /*gamemodeTable*/, const GamemodeEventConnection& connection)
	{
		return std::visit([&](auto&& arg)
		{
			using T = std::decay_t<decltype(arg)>;

			if constexpr (std::is_same_v<T, GamemodeEvent>)
			{
				std::size_t eventIndex = static_cast<std::size_t>(arg);
				auto& callbacks = m_eventCallbacks[eventIndex];

				for (auto it = callbacks.begin(); it != callbacks.end(); ++it)
				{
					if (it->callbackId == connection.callbackId)
					{
						callbacks.erase(it);
						return true;
					}
				}

				return false;
			}
			else if constexpr (std::is_same_v<T, std::size_t>)
			{
				if (m_customEventCallbacks.size() <= arg)
					return false;

				auto& callbacks = m_customEventCallbacks[arg];

				for (auto it = callbacks.begin(); it != callbacks.end(); ++it)
				{
					if (it->callbackId == connection.callbackId)
					{
						callbacks.erase(it);
						return true;
					}
				}

				return false;
			}
			else
				static_assert(AlwaysFalse<T>(), "non-exhaustive visitor");
		}, connection.event);
	}
}
