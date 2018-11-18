// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/Scripting/SharedScriptingContext.hpp>
#include <NDK/LuaAPI.hpp>
#include <NDK/Lua/LuaBinding.hpp>
#include <filesystem>
#include <iostream>

namespace bw
{
	SharedScriptingContext::SharedScriptingContext()
	{
		m_luaInstance.LoadLibraries();
		Ndk::LuaAPI::GetBinding()->RegisterClasses(m_luaInstance);

		RegisterMetatableLibrary();

		Load("../../scripts/autorun");
	}

	SharedScriptingContext::~SharedScriptingContext() = default;

	bool SharedScriptingContext::Load(const std::filesystem::path& folderOrFile)
	{
		Nz::LuaInstance& state = GetLuaInstance();

		if (std::filesystem::is_directory(folderOrFile))
		{
			for (auto& p : std::filesystem::directory_iterator(folderOrFile))
				Load(p);
		}
		else if (std::filesystem::is_regular_file(folderOrFile))
		{
			if (state.ExecuteFromFile(folderOrFile.generic_u8string()))
			{
				std::cout << "Loaded " << folderOrFile.generic_u8string() << '\n';
				return true;
			}
			else
			{
				std::cerr << "Failed to load " << folderOrFile.generic_u8string() << ": " << state.GetLastError() << std::endl;
				return false;
			}
		}

		return true;
	}

	void SharedScriptingContext::RegisterMetatableLibrary()
	{
		m_luaInstance.PushFunction([](Nz::LuaState& state) -> int
		{
			const char* metaname = state.CheckString(1);
			if (!state.NewMetatable(metaname))
			{
				state.Pop();
				state.Error("Metatable " + std::string(metaname) + " already exists");
				return 0;
			}

			return 1;
		});
		m_luaInstance.SetGlobal("RegisterMetatable");

		m_luaInstance.PushFunction([](Nz::LuaState& state) -> int
		{
			state.GetMetatable(state.CheckString(1));

			return 1;
		});
		m_luaInstance.SetGlobal("GetMetatable");

		m_luaInstance.PushFunction([](Nz::LuaState& state) -> int
		{
			state.CheckType(1, Nz::LuaType_Table);
			const char* metaname = state.CheckString(2);

			if (!state.GetMetatable(1))
				state.Error("Table has no metatable");

			state.GetMetatable(metaname);

			bool equal = state.RawEqual(-1, -2);

			state.Pop(2);

			if (!equal)
				state.Error("Table is not of type " + std::string(metaname));

			return 1;
		});
		m_luaInstance.SetGlobal("AssertMetatable");
	}
}
