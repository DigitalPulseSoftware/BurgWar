// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/ServerGamemode.hpp>
#include <Nazara/Math/Angle.hpp>
#include <Nazara/Math/Vector2.hpp>
#include <NDK/Components.hpp>
#include <CoreLib/Match.hpp>
#include <CoreLib/Terrain.hpp>
#include <cassert>

namespace bw
{
	ServerGamemode::ServerGamemode(Match& match, std::shared_ptr<ScriptingContext> scriptingContext, std::filesystem::path gamemodePath) :
	SharedGamemode(match, std::move(scriptingContext), std::move(gamemodePath)),
	m_match(match)
	{
		InitializeGamemode();
	}

	void ServerGamemode::Reload()
	{
		SharedGamemode::Reload();

		InitializeGamemode();
	}

	void ServerGamemode::InitializeGamemode()
	{
		auto& context = GetScriptingContext();

		auto Load = [&](const std::filesystem::path& filepath)
		{
			return context->Load(filepath);
		};

		sol::state& state = context->GetLuaState();
		state["GM"] = GetGamemodeTable();

		const std::filesystem::path& gamemodePath = GetGamemodePath();
		Load(gamemodePath / "shared.lua");
		Load(gamemodePath / "sv_init.lua");
	}
}
