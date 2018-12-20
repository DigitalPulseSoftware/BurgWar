// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/Scripting/ServerScriptingContext.hpp>
#include <Shared/Match.hpp>
#include <iostream>

namespace bw
{
	ServerScriptingContext::ServerScriptingContext(Match& match) :
	SharedScriptingContext(true),
	m_match(match)
	{
		Nz::LuaState& state = GetLuaState();
		state.PushFunction([&](Nz::LuaState& state) -> int
		{
			std::string path = state.CheckString(1);

			m_match.RegisterClientScript(GetCurrentFolder() / path);

			return 0;
		});
		state.SetGlobal("RegisterClientScript");

		RegisterLibrary();
	}
}
