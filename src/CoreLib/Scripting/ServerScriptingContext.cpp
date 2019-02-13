// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/ServerScriptingContext.hpp>
#include <CoreLib/Match.hpp>
#include <iostream>

namespace bw
{
	bool ServerScriptingContext::Load(const std::filesystem::path& folderOrFile)
	{
		sol::state& state = GetLuaState();

		if (std::filesystem::is_directory(folderOrFile))
		{
			for (auto& p : std::filesystem::directory_iterator(folderOrFile))
				Load(p);

			return true;
		}
		else if (std::filesystem::is_regular_file(folderOrFile))
		{
			m_currentFolder = folderOrFile.parent_path();

			auto result = state.do_file(folderOrFile.generic_u8string());
			if (result.valid())
			{
				std::cout << "Loaded " << folderOrFile << std::endl;
				return true;
			}
			else
			{
				sol::error err = result;
				std::cerr << "Failed to load " << folderOrFile.generic_u8string() << ": " << err.what() << std::endl;
				return false;
			}
		}

		std::cerr << "Unknown path " << folderOrFile.generic_u8string() << std::endl;
		return false;
	}
}