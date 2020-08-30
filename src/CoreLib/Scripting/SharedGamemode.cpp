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
	SharedGamemode::SharedGamemode(SharedMatch& match, std::shared_ptr<ScriptingContext> scriptingContext, std::string gamemodeName) :
	m_context(std::move(scriptingContext)),
	m_gamemodeName(std::move(gamemodeName)),
	m_sharedMatch(match)
	{
	}

	void SharedGamemode::InitializeGamemode()
	{
		auto resultOpt = m_context->Load("gamemodes/" + m_gamemodeName + ".lua");
		if (!resultOpt)
			throw std::runtime_error("failed to retrieve gamemode " + m_gamemodeName + " data");

		m_gamemodeTable = resultOpt->as<sol::table>();

		m_gamemodeTable["On"] = [&](const sol::table& gamemodeTable, const std::string_view& event, sol::protected_function callback)
		{
			RegisterEvent(gamemodeTable, event, std::move(callback), false);
		};

		m_gamemodeTable["OnAsync"] = [&](const sol::table& gamemodeTable, const std::string_view& event, sol::protected_function callback)
		{
			RegisterEvent(gamemodeTable, event, std::move(callback), true);
		};

		sol::state& state = m_context->GetLuaState();
		state["GM"] = GetGamemodeTable();
	}

	void SharedGamemode::RegisterEvent(const sol::table& gamemodeTable, const std::string_view& event, sol::protected_function callback, bool async)
	{
		assert(gamemodeTable == m_gamemodeTable);

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
