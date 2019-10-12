// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/SharedScriptingLibrary.hpp>
#include <CoreLib/Components/ScriptComponent.hpp>
#include <CoreLib/Scripting/ScriptingContext.hpp>
#include <CoreLib/SharedMatch.hpp>

namespace bw
{
	SharedScriptingLibrary::SharedScriptingLibrary(SharedMatch& sharedMatch) :
	AbstractScriptingLibrary(sharedMatch.GetLogger()),
	m_match(sharedMatch)
	{
	}

	SharedScriptingLibrary::~SharedScriptingLibrary() = default;

	void SharedScriptingLibrary::RegisterLibrary(ScriptingContext& context)
	{
		sol::state& luaState = context.GetLuaState();
		luaState.open_libraries();

		RegisterGlobalLibrary(context);
		RegisterMatchLibrary(context);
		RegisterMetatableLibrary(context);
		RegisterTimerLibrary(context);
	}

	void SharedScriptingLibrary::RegisterMatchLibrary(ScriptingContext& context)
	{
		sol::state& state = context.GetLuaState();
		sol::table script = state.create_table();

		script["GetCurrentTime"] = [this]()
		{
			return m_match.GetCurrentTime() / 1000.f;
		};

		state["match"] = script;
	}

	void SharedScriptingLibrary::RegisterTimerLibrary(ScriptingContext& context)
	{
		sol::state& luaState = context.GetLuaState();
		luaState["engine_SetTimer"] = [&](Nz::UInt64 time, sol::object callbackObject)
		{
			m_match.GetTimerManager().PushCallback(m_match.GetCurrentTime() + time, [this, &luaState, callbackObject]()
			{
				sol::protected_function callback(luaState, sol::ref_index(callbackObject.registry_index()));

				auto result = callback();
				if (!result.valid())
				{
					sol::error err = result;
					bwLog(GetLogger(), LogLevel::Error, "engine_SetTimer failed: {0}", err.what());
				}
			});
		};

		luaState["GetEntitiesByClass"] = [&](sol::this_state s, const std::string& entityClass)
		{
			sol::state_view state(s);
			sol::table result = state.create_table();

			std::size_t index = 1;
			m_match.ForEachEntity([&](const Ndk::EntityHandle& entity)
			{
				if (!entity->HasComponent<ScriptComponent>())
					return;

				auto& entityScript = entity->GetComponent<ScriptComponent>();
				if (entityScript.GetElement()->fullName == entityClass)
					result[index++] = entityScript.GetTable();
			});

			return result;
		};
	}
}
