// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <GameLibShared/Scripting/SharedScriptingContext.hpp>
#include <GameLibShared/SharedMatch.hpp>
#include <filesystem>
#include <iostream>

namespace bw
{
	SharedScriptingContext::SharedScriptingContext(SharedMatch& sharedMatch, bool isServer) :
	m_match(sharedMatch)
	{
		m_luaState.open_libraries();

		m_luaState["SERVER"] = isServer;
		m_luaState["CLIENT"] = !isServer;
	}

	SharedScriptingContext::~SharedScriptingContext()
	{
		m_availableThreads.clear();
		m_runningThreads.clear();
	}

	void SharedScriptingContext::Update()
	{
		for (auto it = m_runningThreads.begin(); it != m_runningThreads.end();)
		{
			sol::thread& runningThread = *it;
			lua_State* lthread = runningThread.thread_state();

			bool removeThread = true;
			switch (static_cast<sol::thread_status>(lua_status(lthread)))
			{
				case sol::thread_status::ok:
					// Coroutine has finished without error, we can recycle its thread
					m_availableThreads.emplace_back(std::move(runningThread));
					break;

				case sol::thread_status::yielded:
					removeThread = false;
					break;

				// Errors
				case sol::thread_status::dead:
				case sol::thread_status::handler:
				case sol::thread_status::gc:
				case sol::thread_status::memory:
				case sol::thread_status::runtime:
					break;
			}

			if (removeThread)
				it = m_runningThreads.erase(it);
			else
				++it;
		}
	}

	void SharedScriptingContext::RegisterLibrary()
	{
		RegisterGlobalLibrary();
		RegisterMetatableLibrary();

		// Shame :bell:
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

		m_luaState["engine_SetTimer"] = [&](Nz::UInt64 time, sol::object callbackObject)
		{
			m_match.GetTimerManager().PushCallback(time, [this, callbackObject]()
			{
				sol::protected_function callback(GetLuaState(), sol::ref_index(callbackObject.registry_index()));

				auto result = callback();
				if (!result.valid())
				{
					sol::error err = result;
					std::cerr << "engine_SetTimer callback failed: " << err.what() << std::endl;
				}
			});
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
