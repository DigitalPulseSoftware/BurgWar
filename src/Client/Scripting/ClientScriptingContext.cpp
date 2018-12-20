// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/Scripting/ClientScriptingContext.hpp>
#include <iostream>

namespace bw
{
	ClientScriptingContext::ClientScriptingContext(LocalMatch& match) :
	SharedScriptingContext(false),
	m_match(match)
	{
		sol::state& state = GetLuaState();
		state["RegisterClientScript"] = []() {}; // Dummy function

		RegisterLibrary();
	}

	/*bool ClientScriptingContext::LoadFile(const std::filesystem::path& filePath, std::string* error)
	{
		return false;
	}*/
}
