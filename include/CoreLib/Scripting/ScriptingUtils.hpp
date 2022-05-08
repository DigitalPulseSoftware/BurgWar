// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SCRIPTING_UTILS_HPP
#define BURGWAR_CORELIB_SCRIPTING_UTILS_HPP

#include <CoreLib/Export.hpp>
#include <entt/entt.hpp>
#include <sol/sol.hpp>
#include <cstdlib>
#include <memory>
#include <optional>

namespace bw
{
	struct ScriptedElement;

	BURGWAR_CORELIB_API std::shared_ptr<ScriptedElement> AssertScriptElement(const sol::table& entityTable);
	BURGWAR_CORELIB_API entt::entity AssertScriptEntity(entt::registry& registry, const sol::table& entityTable);
	BURGWAR_CORELIB_API std::shared_ptr<ScriptedElement> RetrieveScriptElement(const sol::table& entityTable);
	BURGWAR_CORELIB_API entt::entity RetrieveScriptEntity(const sol::table& entityTable);

	BURGWAR_CORELIB_API std::optional<sol::object> TranslateEntityToLua(entt::registry& registry, entt::entity entity);
	template<typename... Args> [[noreturn]] void TriggerLuaError(lua_State* L, const char* format, Args&&... args);
	[[noreturn]] BURGWAR_CORELIB_API void TriggerLuaError(lua_State* L, const std::string& errMessage);
	[[noreturn]] BURGWAR_CORELIB_API void TriggerLuaArgError(lua_State* L, int argIndex, const char* errMessage);
	[[noreturn]] BURGWAR_CORELIB_API void TriggerLuaArgError(lua_State* L, int argIndex, const std::string& errMessage);

	template<typename F> auto LuaFunction(F funcPtr);
}

#include <CoreLib/Scripting/ScriptingUtils.inl>

#endif
