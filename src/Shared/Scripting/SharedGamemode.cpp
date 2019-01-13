// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/Scripting/SharedGamemode.hpp>
#include <Nazara/Math/Vector2.hpp>
#include <NDK/Components.hpp>
#include <Shared/Match.hpp>
#include <Shared/Terrain.hpp>
#include <cassert>
#include <iostream>

namespace bw
{
	SharedGamemode::SharedGamemode(std::shared_ptr<SharedScriptingContext> scriptingContext, std::filesystem::path gamemodePath) :
	m_gamemodePath(std::move(gamemodePath)),
	m_context(std::move(scriptingContext))
	{
		sol::state& state = m_context->GetLuaState();

		m_gamemodeTable = state.create_table();
	}
}
