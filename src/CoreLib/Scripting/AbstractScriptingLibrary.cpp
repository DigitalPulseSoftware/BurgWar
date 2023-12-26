// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/AbstractScriptingLibrary.hpp>
#include <CoreLib/Utils.hpp>
#include <CoreLib/Components/OwnerComponent.hpp>
#include <CoreLib/Components/ScriptComponent.hpp>
#include <CoreLib/LogSystem/Logger.hpp>
#include <CoreLib/Scripting/RandomEngine.hpp>
#include <CoreLib/Scripting/ScriptingContext.hpp>
#include <CoreLib/Scripting/ScriptingUtils.hpp>

namespace bw
{
	namespace
	{
		int PushAsString(lua_State* L, int idx, bool extended);
		int PushTableAsString(lua_State* L);

		int PushAsString(lua_State* L, int idx, bool extended)
		{
			if (!luaL_callmeta(L, idx, "__tostring"))
			{
				int t = lua_type(L, idx);

				switch (t)
				{
					case LUA_TNIL:
						lua_pushliteral(L, "nil");
						break;

					case LUA_TSTRING:
						lua_pushvalue(L, idx);
						break;

					case LUA_TNUMBER:
						if (lua_isinteger(L, idx))
							lua_pushfstring(L, "%I", (LUAI_UACINT)lua_tointeger(L, idx));
						else
							lua_pushfstring(L, "%f", (LUAI_UACNUMBER)lua_tonumber(L, idx));
						break;

					case LUA_TBOOLEAN:
						if (lua_toboolean(L, idx))
							lua_pushliteral(L, "true");
						else
							lua_pushliteral(L, "false");
						break;

					case LUA_TTABLE:
					{
						if (extended)
						{
							lua_pushvalue(L, idx);
							lua_pushliteral(L, "");
							lua_createtable(L, 0, 0);
							PushTableAsString(L);
							break;
						}

						[[fallthrough]];
					}

					default:
					{
						int tt = luaL_getmetafield(L, idx, "__name");
						const char* name = (tt == LUA_TSTRING) ? lua_tostring(L, idx) : lua_typename(L, t);
						lua_pushfstring(L, "%s: %p", name, lua_topointer(L, idx));
						if (tt != LUA_TNIL)
							lua_replace(L, -2);
						break;
					}
				}
			}
			else
			{
				if (!lua_isstring(L, -1))
					luaL_error(L, "'__tostring' must return a string");
			}

			return 1;
		}

		int PushTableAsString(lua_State* L)
		{
			int tableIndex = lua_absindex(L, -3);
			int indentIndex = lua_absindex(L, -2);
			int visitedIndex = lua_absindex(L, -1);

			luaL_checktype(L, tableIndex, LUA_TTABLE); //< table
			luaL_checktype(L, indentIndex, LUA_TSTRING); //< indentation
			luaL_checktype(L, visitedIndex, LUA_TTABLE); //< visited

			// Add indentation
			lua_pushvalue(L, indentIndex);
			lua_pushstring(L, "  ");
			lua_concat(L, 2);
			int newIndentIndex = lua_absindex(L, -1); //< new value on top of the stack is the new indentation

			luaL_Buffer b;
			luaL_buffinit(L, &b);

			lua_pushvalue(L, indentIndex);
			luaL_addvalue(&b);

			lua_pushstring(L, "{\n");
			luaL_addvalue(&b);

			lua_pushnil(L);
			while (lua_next(L, tableIndex))
			{
				bool recursive = false;
				if (lua_istable(L, -1))
				{
					lua_pushvalue(L, -1);
					lua_gettable(L, visitedIndex);
					if (lua_isnil(L, -1))
					{
						lua_pop(L, 1);

						lua_pushvalue(L, -1);
						lua_pushboolean(L, true);
						lua_settable(L, visitedIndex);

						recursive = true;
					}
				}

				// Indent
				lua_pushvalue(L, newIndentIndex);
				luaL_addvalue(&b);

				// Key
				PushAsString(L, -2, false);
				luaL_addvalue(&b);

				// = 
				lua_pushstring(L, (recursive) ? " = \n" : " = ");
				luaL_addvalue(&b);

				// Value
				if (recursive)
				{
					lua_pushvalue(L, -1);
					lua_pushvalue(L, newIndentIndex);
					lua_pushvalue(L, visitedIndex);
					PushTableAsString(L);
				}
				else
					PushAsString(L, -1, false);

				luaL_addvalue(&b);

				lua_pushstring(L, "\n");
				luaL_addvalue(&b);

				lua_pop(L, 1);
			}

			lua_pop(L, 2); // pop new indentation and visited table

			luaL_addvalue(&b); //< add (and pop) new indentation to the buffer

			lua_pushstring(L, "}");
			luaL_addvalue(&b);

			lua_pop(L, 1); //< pop table

			luaL_pushresult(&b);
			return 1;
		}
	}

	AbstractScriptingLibrary::~AbstractScriptingLibrary() = default;

	void AbstractScriptingLibrary::RegisterGlobalLibrary(ScriptingContext& context)
	{
		sol::state& luaState = context.GetLuaState();
		luaState["include"] = LuaFunction([&](sol::this_state L, const std::string& scriptName)
		{
			std::filesystem::path scriptPath = context.GetCurrentFolder() / Nz::Utf8Path(scriptName);

			auto result = context.Load(scriptPath);
			if (!result)
				TriggerLuaError(L, result.error());
		});

		luaState["tostring"] = [](lua_State* L) -> int
		{
			luaL_checkany(L, 1);
			bool extended = luaL_opt(L, lua_toboolean, 2, false);
			PushAsString(L, 1, extended);
			return 1;
		};

		auto PrintExt = [&context](sol::this_state L, sol::variadic_args args, bool extended)
		{
			bool first = true;

			std::ostringstream oss;
			for (auto v : args)
			{
				PushAsString(L, v.stack_index(), extended);

				std::size_t length;
				const char* str = lua_tolstring(L, -1, &length);
				oss << std::string(str, length);
				if (!first)
					oss << "\t";

				first = false;
			}

			context.Print(oss.str());
		};

		luaState["pprint"] = LuaFunction([=](sol::this_state L, sol::variadic_args args)
		{
			PrintExt(L, args, true);
		});

		luaState["print"] = LuaFunction([=](sol::this_state L, sol::variadic_args args)
		{
			PrintExt(L, args, false);
		});
	}

	void AbstractScriptingLibrary::RegisterMetatableLibrary(ScriptingContext& context)
	{
		sol::state& luaState = context.GetLuaState();
		luaState["RegisterMetatable"] = LuaFunction([](sol::this_state L, const char* metaname)
		{
			if (luaL_newmetatable(L, metaname) == 0)
			{
				lua_pop(L, 1);
				TriggerLuaError(L, "Metatable %s already exists", metaname);
			}

			return sol::stack_table(L);
		});

		luaState["GetMetatable"] = LuaFunction([](sol::this_state L, const char* metaname)
		{
			luaL_getmetatable(L, metaname);
			return sol::stack_table(L);
		});

		luaState["AssertMetatable"] = LuaFunction([](sol::this_state L, sol::table tableRef, const char* metaname)
		{
			sol::table metatable = tableRef[sol::metatable_key];
			if (!metatable)
				throw std::runtime_error("Table has no metatable");

			luaL_getmetatable(L, metaname);
			sol::stack_table expectedMetatable;

			metatable.push();
			bool equal = lua_rawequal(L, expectedMetatable.stack_index(), -1);

			lua_pop(L, 2);

			if (!equal)
				TriggerLuaError(L, "Table is not of type %s", metaname);

			return tableRef;
		});
	}

	sol::usertype<RandomEngine> AbstractScriptingLibrary::RegisterRandomEngineClass(ScriptingContext& context)
	{
		sol::state& lua = context.GetLuaState();
		return lua.new_usertype<RandomEngine>("RandomEngine",
			sol::constructors<RandomEngine(), RandomEngine(std::uint_fast64_t)>(),

			"Generate", sol::overload(
				LuaFunction([](RandomEngine& engine) { return engine.Generate(); }),
				LuaFunction([](RandomEngine& engine, Nz::Int64 a, Nz::Int64 b) { return engine.Generate(a, b); })
			),

			"Seed", &RandomEngine::Seed
		);
	}
}
