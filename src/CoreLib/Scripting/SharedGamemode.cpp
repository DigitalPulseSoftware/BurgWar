// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/SharedGamemode.hpp>
#include <CoreLib/Components/HealthComponent.hpp>
#include <Nazara/Math/Vector2.hpp>
#include <NDK/Components.hpp>
#include <NDK/Systems.hpp>
#include <CoreLib/Match.hpp>
#include <CoreLib/Terrain.hpp>
#include <cassert>

namespace bw
{
	SharedGamemode::SharedGamemode(SharedMatch& match, std::shared_ptr<ScriptingContext> scriptingContext, std::string gamemodeName, PropertyValueMap propertyValues) :
	m_context(std::move(scriptingContext)),
	m_gamemodeName(std::move(gamemodeName)),
	m_propertyValues(std::move(propertyValues)),
	m_sharedMatch(match)
	{
	}

	SharedGamemode::~SharedGamemode() = default;

	void SharedGamemode::Reload()
	{
		InitializeMetatable();
		m_gamemodeTable = LoadGamemode(m_gamemodeName, nullptr);
	}

	sol::table SharedGamemode::LoadGamemode(const std::string& gamemodeName, std::size_t* newPropertyIndex)
	{
		auto resultOpt = m_context->Load("gamemodes/" + gamemodeName + ".lua");
		if (!resultOpt)
			throw std::runtime_error("failed to retrieve gamemode " + gamemodeName + " data");

		sol::table gamemodeTable = resultOpt->as<sol::table>();
		gamemodeTable["__index"] = gamemodeTable;

		std::size_t propertyIndex = 0;

		std::string baseGamemode = gamemodeTable.get_or("Base", std::string{});
		if (!baseGamemode.empty())
		{
			sol::table parentGamemodeTable = LoadGamemode(baseGamemode, &propertyIndex);
			gamemodeTable["Base"] = parentGamemodeTable;
			gamemodeTable[sol::metatable_key] = parentGamemodeTable;
		}
		else
			gamemodeTable[sol::metatable_key] = m_gamemodeMetatable;

		sol::object properties = gamemodeTable.get<sol::object>("Properties");
		if (properties)
		{
			sol::table elementProperties = properties.as<sol::table>();

			for (const auto& kv : elementProperties)
			{
				sol::table propertyTable = kv.second;

				std::string propertyName = propertyTable["Name"];

				try
				{
					ScriptedProperty property = InitPropertyFromLua(propertyIndex, propertyTable);

					auto it = m_properties.find(propertyName);
					if (it == m_properties.end())
						m_properties.emplace(std::move(propertyName), std::move(property));
					else
						throw std::runtime_error("Property " + propertyName + " already exists");

					propertyIndex++;
				}
				catch (const std::exception& e)
				{
					bwLog(m_sharedMatch.GetLogger(), LogLevel::Error, "Failed to load property {0} for gamemode {1}: {2}", propertyName, m_gamemodeName, e.what());
				}
			}

			if (newPropertyIndex)
				*newPropertyIndex = propertyIndex;
		}

		sol::state& state = m_context->GetLuaState();
		state["ScriptedGamemode"] = [&]()
		{
			return gamemodeTable;
		};

		InitializeGamemode(gamemodeName);

		state["ScriptedGamemode"] = sol::nil;

		return gamemodeTable;
	}

	void SharedGamemode::InitializeMetatable()
	{
		sol::state& state = m_context->GetLuaState();
		m_gamemodeMetatable = state.create_table();
		m_gamemodeMetatable["__index"] = m_gamemodeMetatable;

		m_gamemodeMetatable["On"] = [&](const sol::table& gamemodeTable, const std::string_view& event, sol::main_protected_function callback)
		{
			RegisterEvent(gamemodeTable, event, std::move(callback), false);
		};

		m_gamemodeMetatable["OnAsync"] = [&](const sol::table& gamemodeTable, const std::string_view& event, sol::main_protected_function callback)
		{
			RegisterEvent(gamemodeTable, event, std::move(callback), true);
		};

		m_gamemodeMetatable["GetProperty"] = [&](sol::this_state s, const sol::table& /*table*/, const std::string& propertyName) -> sol::object
		{
			auto propertyVal = GetProperty(propertyName);
			if (propertyVal.has_value())
			{
				sol::state_view lua(s);
				const PropertyValue& property = propertyVal.value();

				auto propertyIt = m_properties.find(propertyName);
				assert(propertyIt != m_properties.end());

				return TranslatePropertyToLua(&m_sharedMatch, lua, property, propertyIt->second.type);
			}
			else
				return sol::nil;
		};
	}

	void SharedGamemode::RegisterEvent(const sol::table& /*gamemodeTable*/, const std::string_view& event, sol::main_protected_function callback, bool async)
	{
		std::optional<GamemodeEvent> gamemodeEventOpt = RetrieveGamemodeEvent(event);
		if (!gamemodeEventOpt)
			throw std::runtime_error("unknown event " + std::string(event));

		GamemodeEvent scriptingEvent = gamemodeEventOpt.value();
		std::size_t eventIndex = static_cast<std::size_t>(scriptingEvent);

		if (async && HasReturnValue(scriptingEvent))
			throw std::runtime_error("events returning a value cannot be async");

		auto& callbackData = m_eventCallbacks[eventIndex].emplace_back();
		callbackData.async = async;
		callbackData.callback = std::move(callback);
	}
}
