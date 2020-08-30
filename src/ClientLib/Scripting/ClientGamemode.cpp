// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Scripting/ClientGamemode.hpp>
#include <ClientLib/LocalMatch.hpp>
#include <cassert>

namespace bw
{
	ClientGamemode::ClientGamemode(LocalMatch& localMatch, std::shared_ptr<ScriptingContext> scriptingContext, std::string gamemodeName, PropertyValueMap propertyValues) :
	SharedGamemode(localMatch, std::move(scriptingContext), std::move(gamemodeName), std::move(propertyValues)),
	m_match(localMatch)
	{
		InitializeGamemode();
	}

	void ClientGamemode::Reload()
	{
		InitializeGamemode();
	}

	void ClientGamemode::InitializeGamemode()
	{
		SharedGamemode::InitializeGamemode();

		auto& context = GetScriptingContext();

		auto Load = [&](const std::filesystem::path& filepath)
		{
			return context->Load(filepath);
		};

		std::filesystem::path gamemodeName = GetGamemodeName();
		Load("gamemodes" / gamemodeName / "shared.lua");
		Load("gamemodes" / gamemodeName / "cl_init.lua");
	}
}
