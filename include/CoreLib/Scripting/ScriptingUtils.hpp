// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SCRIPTING_UTILS_HPP
#define BURGWAR_CORELIB_SCRIPTING_UTILS_HPP

#include <NDK/Entity.hpp>
#include <Thirdparty/sol3/sol.hpp>
#include <cstdlib>
#include <memory>

namespace bw
{
	struct ScriptedElement;

	std::shared_ptr<ScriptedElement> AssertScriptElement(const sol::table& entityTable);
	Ndk::EntityHandle AssertScriptEntity(const sol::table& entityTable);
	std::shared_ptr<ScriptedElement> RetrieveScriptElement(const sol::table& entityTable);
	Ndk::EntityHandle RetrieveScriptEntity(const sol::table& entityTable);

	sol::object TranslateEntityToLua(const Ndk::EntityHandle& entity);
	template<typename... Args> [[noreturn]] void TriggerLuaError(lua_State* L, const char* format, Args&&... args);
	[[noreturn]] void TriggerLuaError(lua_State* L, const std::string& errMessage);
	[[noreturn]] void TriggerLuaArgError(lua_State* L, int argIndex, const char* errMessage);
	[[noreturn]] void TriggerLuaArgError(lua_State* L, int argIndex, const std::string& errMessage);

	template<typename F> auto ExceptToLuaErr(F funcPtr);
}

#include <CoreLib/Scripting/ScriptingUtils.inl>

#endif
