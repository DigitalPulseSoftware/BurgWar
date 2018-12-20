// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/Scripting/SharedScriptingContext.hpp>
#include <Nazara/Lua/LuaCoroutine.hpp>
#include <NDK/LuaAPI.hpp>
#include <NDK/Lua/LuaBinding.hpp>
#include <filesystem>
#include <iostream>

namespace bw
{
	SharedScriptingContext::SharedScriptingContext(bool isServer)
	{
		m_luaInstance.LoadLibraries();
		Ndk::LuaAPI::GetBinding()->RegisterClasses(m_luaInstance);

		m_luaInstance.PushGlobal("SERVER", isServer);
		m_luaInstance.PushGlobal("CLIENT", !isServer);
	}

	SharedScriptingContext::~SharedScriptingContext()
	{
		// Destroy coroutines before destroying lua instance
		m_coroutines.clear();
	}

	Nz::LuaCoroutine& SharedScriptingContext::CreateCoroutine()
	{
		return m_coroutines.emplace_back(m_luaInstance.NewCoroutine());
	}

	bool SharedScriptingContext::Load(const std::filesystem::path& folderOrFile)
	{
		Nz::LuaInstance& state = GetLuaState();

		if (std::filesystem::is_directory(folderOrFile))
		{
			for (auto& p : std::filesystem::directory_iterator(folderOrFile))
				Load(p);

			return true;
		}
		else if (std::filesystem::is_regular_file(folderOrFile))
		{
			m_currentFolder = folderOrFile.parent_path();

			std::string err;
			if (LoadFile(folderOrFile, &err))
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

	void SharedScriptingContext::Update()
	{
		for (auto it = m_coroutines.begin(); it != m_coroutines.end();)
		{
			// Is coroutine dead/finished?
			if (!it->CanResume())
				it = m_coroutines.erase(it);
			else
				++it;
		}
	}

	bool SharedScriptingContext::LoadFile(const std::filesystem::path& filePath, std::string* error)
	{
		Nz::LuaInstance& state = GetLuaState();

		if (state.ExecuteFromFile(filePath.generic_u8string()))
			return true;
		else
		{
			*error = state.GetLastError().ToStdString();
			return false;
		}
	}

	void SharedScriptingContext::RegisterLibrary()
	{
		RegisterGlobalLibrary();
		RegisterMetatableLibrary();

		Load("../../scripts/autorun");
	}

	void SharedScriptingContext::RegisterGlobalLibrary()
	{
		m_luaInstance.PushFunction([&](Nz::LuaState& state) -> int
		{
			int index = 1;
			std::string scriptName = state.Check<std::string>(&index);

			std::filesystem::path scriptPath = m_currentFolder / scriptName;

			if (!state.ExecuteFromFile(scriptPath.generic_u8string()))
				state.Error(state.GetLastError());

			return 0;
		});
		m_luaInstance.SetGlobal("include");
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
