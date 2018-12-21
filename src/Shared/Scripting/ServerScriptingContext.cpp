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
		Nz::LuaState& state = GetLuaInstance();
		state.PushFunction([&](Nz::LuaState& state) -> int
		{
			std::string path = state.CheckString(1);

			m_match.RegisterClientScript(GetCurrentFolder() / path);

			return 0;
		});
		state.SetGlobal("RegisterClientScript");

		RegisterLibrary();
	}

	bool ServerScriptingContext::Load(const std::filesystem::path& folderOrFile)
	{
		Nz::LuaInstance& state = GetLuaInstance();

		if (std::filesystem::is_directory(folderOrFile))
		{
			for (auto& p : std::filesystem::directory_iterator(folderOrFile))
				Load(p);

			return true;
		}
		else if (std::filesystem::is_regular_file(folderOrFile))
		{
			m_currentFolder = folderOrFile.parent_path();

			if (state.ExecuteFromFile(folderOrFile.generic_u8string()))
			{
				std::cout << "Loaded " << folderOrFile << std::endl;
				return true;
			}
			else
			{
				std::cerr << "Failed to load " << folderOrFile.generic_u8string() << ": " << state.GetLastError() << std::endl;
				return false;
			}
		}

		std::cerr << "Unknown path " << folderOrFile.generic_u8string() << std::endl;
		return false;
	}
}
