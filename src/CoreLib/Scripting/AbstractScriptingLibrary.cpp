// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/AbstractScriptingLibrary.hpp>
#include <CoreLib/Components/OwnerComponent.hpp>
#include <CoreLib/Components/ScriptComponent.hpp>
#include <CoreLib/LogSystem/Logger.hpp>
#include <CoreLib/Scripting/ScriptingContext.hpp>

namespace bw
{
	AbstractScriptingLibrary::~AbstractScriptingLibrary() = default;

	void AbstractScriptingLibrary::RegisterGlobalLibrary(ScriptingContext& context)
	{
		sol::state& luaState = context.GetLuaState();
		luaState["include"] = [&](const std::string& scriptName)
		{
			std::filesystem::path scriptPath = context.GetCurrentFolder() / scriptName;

			if (!context.Load(scriptPath.generic_u8string()))
				throw std::runtime_error("TODO");
		};

		luaState["print"] = [this](sol::this_state L, sol::variadic_args args)
		{
			bool first = true;

			std::ostringstream oss;
			for (auto v : args)
			{
				std::size_t length;
				const char* str = luaL_tolstring(L, v.stack_index(), &length);
				oss << std::string(str, length);
				if (!first)
					oss << "\t";

				first = false;
			}

			bwLog(GetLogger(), LogLevel::Info, oss.str());
		};
	}

	void AbstractScriptingLibrary::RegisterMetatableLibrary(ScriptingContext& context)
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
