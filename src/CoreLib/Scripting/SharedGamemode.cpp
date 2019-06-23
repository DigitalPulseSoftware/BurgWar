// Copyright (C) 2019 Jérôme Leclercq
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
#include <iostream>

namespace bw
{
	SharedGamemode::SharedGamemode(SharedMatch& match, std::shared_ptr<ScriptingContext> scriptingContext, std::filesystem::path gamemodePath) :
	m_gamemodePath(std::move(gamemodePath)),
	m_context(std::move(scriptingContext)),
	m_sharedMatch(match)
	{
		sol::state& state = m_context->GetLuaState();

		m_gamemodeTable = state.create_table();

		InitializeGamemode();
	}

	void SharedGamemode::InitializeGamemode()
	{
		m_gamemodeTable["GetTickDuration"] = [&](const sol::table& gmTable)
		{
			return m_sharedMatch.GetTickDuration();
		};
	}
}
