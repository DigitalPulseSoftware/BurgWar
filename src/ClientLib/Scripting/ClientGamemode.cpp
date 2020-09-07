// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Scripting/ClientGamemode.hpp>
#include <ClientLib/LocalMatch.hpp>
#include <cassert>

namespace bw
{
	ClientGamemode::ClientGamemode(LocalMatch& localMatch, std::shared_ptr<ScriptingContext> scriptingContext, std::string gamemodeName, PropertyValueMap propertyValues) :
	SharedGamemode(localMatch, std::move(scriptingContext), std::move(gamemodeName), std::move(propertyValues))
	{
		Reload();
	}

	void ClientGamemode::InitializeGamemode(const std::string& gamemodeName)
	{
		const std::filesystem::path gamemodeFolder = "gamemodes";

		std::filesystem::path gamemodePath = gamemodeFolder / gamemodeName;

		auto& context = GetScriptingContext();
		context->Load(gamemodePath / "shared.lua");
		context->Load(gamemodePath / "cl_init.lua");
	}
}
