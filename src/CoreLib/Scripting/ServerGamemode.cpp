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
		Reload();
	}

	void ServerGamemode::InitializeGamemode(const std::string& gamemodeName)
	{
		const std::filesystem::path gamemodeFolder = "gamemodes";

		std::filesystem::path gamemodePath = gamemodeFolder / gamemodeName;

		auto& context = GetScriptingContext();
		context->Load(gamemodePath / "shared.lua");
		context->Load(gamemodePath / "sv_init.lua");

		m_match.RegisterClientScript(gamemodePath.generic_u8string() + ".lua");
	}
}
