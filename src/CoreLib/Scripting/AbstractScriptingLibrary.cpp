// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/AbstractScriptingLibrary.hpp>
#include <CoreLib/Scripting/SharedScriptingContext.hpp>
#include <iostream>

namespace bw
{
	AbstractScriptingLibrary::~AbstractScriptingLibrary() = default;

	void AbstractScriptingLibrary::RegisterGlobalLibrary(SharedScriptingContext& context)
	{
		sol::state& luaState = context.GetLuaState();
		luaState["include"] = [&](const std::string& scriptName)
		{
			std::filesystem::path scriptPath = context.GetCurrentFolder() / scriptName;

			if (!context.Load(scriptPath.generic_u8string()))
				throw std::runtime_error("TODO");
		};
	}

	void AbstractScriptingLibrary::RegisterMetatableLibrary(SharedScriptingContext& context)
	{
		sol::state& luaState = context.GetLuaState();
		luaState["RegisterMetatable"] = [](sol::this_state s, const char* metaname)
		{
			if (luaL_newmetatable(s, metaname) == 0)
			{
				lua_pop(s, 1);
				throw std::runtime_error("Metatable " + std::string(metaname) + " already exists");
			}

			return sol::stack_table(s);
		};

		luaState["GetMetatable"] = [](sol::this_state s, const char* metaname)
		{
			luaL_getmetatable(s, metaname);
			return sol::stack_table(s);
		};

		luaState["AssertMetatable"] = [](sol::this_state s, sol::table tableRef, const char* metaname)
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
