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
	ServerGamemode::ServerGamemode(Match& match, std::shared_ptr<ScriptingContext> scriptingContext, std::string gamemodeName, PropertyValueMap propertyValues) :
	SharedGamemode(match, std::move(scriptingContext), std::move(gamemodeName), std::move(propertyValues)),
	m_match(match)
	{
		InitializeGamemode();
	}

	void ServerGamemode::Reload()
	{
		InitializeGamemode();
	}

	void ServerGamemode::InitializeGamemode()
	{
		SharedGamemode::InitializeGamemode();

		auto& context = GetScriptingContext();

		auto Load = [&](const std::filesystem::path& filepath)
		{
			return context->Load(filepath);
		};

		std::filesystem::path gamemodeName = GetGamemodeName();
		Load("gamemodes" / gamemodeName / "shared.lua");
		Load("gamemodes" / gamemodeName / "sv_init.lua");

		std::filesystem::path gamemodeFile = "gamemodes" / gamemodeName;
		gamemodeFile.replace_extension("lua");

		m_match.RegisterClientScript(gamemodeFile);
	}
}
