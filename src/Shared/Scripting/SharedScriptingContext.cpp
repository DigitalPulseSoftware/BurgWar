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
	SharedScriptingContext::SharedScriptingContext(bool isServer)
	{
		m_luaState.open_libraries();

		m_luaState["SERVER"] = isServer;
		m_luaState["CLIENT"] = !isServer;
	}

	SharedScriptingContext::~SharedScriptingContext()
	{
		// Destroy coroutines before destroying lua instance
		m_coroutines.clear();
	}

	void SharedScriptingContext::Update()
	{
		for (auto it = m_coroutines.begin(); it != m_coroutines.end();)
		{
			// Is coroutine dead/finished?
			if (!it->runnable())
				it = m_coroutines.erase(it);
			else
				++it;
		}
	}

	void SharedScriptingContext::RegisterLibrary()
	{
		RegisterGlobalLibrary();
		RegisterMetatableLibrary();

		Load("../../scripts/autorun");
		Load("autorun");
	}

	void SharedScriptingContext::RegisterGlobalLibrary()
	{
		m_luaState["include"] = [&](const std::string& scriptName)
		{
			std::filesystem::path scriptPath = m_currentFolder / scriptName;

			if (!Load(scriptPath.generic_u8string()))
				throw std::runtime_error("TODO");
		};
	}

	void SharedScriptingContext::RegisterMetatableLibrary()
	{
		m_luaState["RegisterMetatable"] = [](sol::this_state s, const char* metaname)
		{
			if (luaL_newmetatable(s, metaname) == 0)
			{
				lua_pop(s, 1);
				throw std::runtime_error("Metatable " + std::string(metaname) + " already exists");
			}

			return sol::stack_table(s);
		};

		m_luaState["GetMetatable"] = [](sol::this_state s, const char* metaname)
		{
			luaL_getmetatable(s, metaname);
			return sol::stack_table(s);
		};

		m_luaState["AssertMetatable"] = [](sol::this_state s, sol::table tableRef, const char* metaname)
		{
			sol::table metatable = tableRef[sol::metatable_key];
			if (!metatable)
				throw std::runtime_error("Table has no metatable");

			luaL_getmetatable(s, metaname);
			sol::stack_table expectedMetatable;

			metatable.push();
			bool equal = lua_rawequal(s, expectedMetatable.stack_index(), -1);

			lua_pop(s, 2);

			if (!metatable)
				throw std::runtime_error("Table is not of type " + std::string(metaname));

			return tableRef;
		};
	}
}
