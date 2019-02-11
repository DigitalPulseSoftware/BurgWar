// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/Scripting/ClientGamemode.hpp>
#include <cassert>
#include <iostream>

namespace bw
{
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

		state["GM"] = nullptr;
	}
}
