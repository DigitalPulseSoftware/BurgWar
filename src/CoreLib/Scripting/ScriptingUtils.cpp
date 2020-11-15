// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/ScriptingUtils.hpp>
#include <CoreLib/Components/ScriptComponent.hpp>

namespace bw
{
	std::shared_ptr<ScriptedElement> AssertScriptElement(const sol::table& entityTable)
	{
		std::shared_ptr<ScriptedElement> element = RetrieveScriptElement(entityTable);
		if (!element)
			TriggerLuaError(entityTable.lua_state(), "invalid element");

		return element;
	}

	Ndk::EntityHandle AssertScriptEntity(const sol::table& entityTable)
	{
		Ndk::EntityHandle entity = RetrieveScriptEntity(entityTable);
		if (!entity || !entity->HasComponent<ScriptComponent>())
			TriggerLuaError(entityTable.lua_state(), "invalid entity");

		return entity;
	}

	std::shared_ptr<ScriptedElement> RetrieveScriptElement(const sol::table& entityTable)
	{
		sol::object entityObject = entityTable["_Element"];
		if (!entityObject)
			return nullptr;

		return entityObject.as<std::shared_ptr<ScriptedElement>>();
	}

	Ndk::EntityHandle RetrieveScriptEntity(const sol::table& entityTable)
	{
		sol::object entityObject = entityTable["_Entity"];
		if (!entityObject)
			return Ndk::EntityHandle::InvalidHandle;

		return entityObject.as<Ndk::EntityHandle>();
	}

	sol::object TranslateEntityToLua(const Ndk::EntityHandle& entity)
	{
		if (!entity || !entity->HasComponent<ScriptComponent>())
			return sol::nil;

		auto& entityScript = entity->GetComponent<ScriptComponent>();
		return entityScript.GetTable();
	}

	[[noreturn]] void TriggerLuaError(lua_State* L, const std::string& errMessage)
	{
		luaL_error(L, errMessage.c_str());
	}

	[[noreturn]] void TriggerLuaArgError(lua_State* L, int argIndex, const char* errMessage)
	{
		luaL_argerror(L, argIndex, errMessage);
	}

	[[noreturn]] void TriggerLuaArgError(lua_State* L, int argIndex, const std::string& errMessage)
	{
		luaL_argerror(L, argIndex, errMessage.c_str());
	}
}
