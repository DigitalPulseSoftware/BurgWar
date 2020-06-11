// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Scripting/ClientGamemode.hpp>
#include <ClientLib/LocalMatch.hpp>
#include <cassert>

namespace bw
{
	ClientGamemode::ClientGamemode(LocalMatch& localMatch, std::shared_ptr<ScriptingContext> scriptingContext, std::filesystem::path gamemodePath) :
	SharedGamemode(localMatch, std::move(scriptingContext), std::move(gamemodePath)),
	m_match(localMatch)
	{
		InitializeGamemode();
	}

	void ClientGamemode::Reload()
	{
		SharedGamemode::Reload();

		InitializeGamemode();
	}

	void ClientGamemode::InitializeGamemode()
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
		Load(gamemodePath / "cl_init.lua");
	}
}
