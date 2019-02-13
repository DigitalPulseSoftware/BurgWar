// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/SharedScriptingContext.hpp>
#include <CoreLib/InputData.hpp>
#include <Nazara/Math/Rect.hpp>
#include <Nazara/Math/Vector2.hpp>
#include <cassert>

namespace bw
{
	template<typename... Args>
	sol::coroutine SharedScriptingContext::CreateCoroutine(Args&&... args)
	{
		auto CreateThread = [&]() -> sol::thread&
		{
			return m_runningThreads.emplace_back(sol::thread::create(m_luaState));
		};

		auto PopThread = [&]() -> sol::thread&
		{
			sol::thread& thread = m_runningThreads.emplace_back(std::move(m_availableThreads.back()));
			m_availableThreads.pop_back();

			return thread;
		};

		sol::thread& thread = (!m_availableThreads.empty()) ? PopThread() : CreateThread();

		return sol::coroutine(thread.state(), std::forward<Args>(args)...);
	}

	inline sol::state& SharedScriptingContext::GetLuaState()
	{
		return m_luaState;
	}

	inline const sol::state& SharedScriptingContext::GetLuaState() const
	{
		return m_luaState;
	}

	inline const std::filesystem::path& SharedScriptingContext::GetCurrentFolder() const
	{
		return m_currentFolder;
	}

	inline SharedMatch& SharedScriptingContext::GetSharedMatch()
	{
		return m_match;
	}
}

namespace sol
{
	template<>
	struct lua_size<bw::InputData> : std::integral_constant<int, 1> {};

	template<>
	struct lua_size<Nz::Rectf> : std::integral_constant<int, 1> {};

	template<>
	struct lua_size<Nz::Vector2f> : std::integral_constant<int, 1> {};

	template<>
	struct lua_type_of<bw::InputData> : std::integral_constant<sol::type, sol::type::table> {};

	template<>
	struct lua_type_of<Nz::Rectf> : std::integral_constant<sol::type, sol::type::table> {};

	template<>
	struct lua_type_of<Nz::Vector2f> : std::integral_constant<sol::type, sol::type::table> {};

	namespace stack
	{
		template<>
		struct checker<Nz::Vector2f>
		{
			template<typename Handler>
			static bool check(lua_State* L, int index, Handler&& handler, record& tracking)
			{
				int absoluteIndex = lua_absindex(L, index);
				bool success = stack::check<sol::table>(L, absoluteIndex, handler);
				if (success)
				{
					luaL_getmetatable(L, "vec2");
					sol::stack_table expectedMetatable;

					lua_getmetatable(L, absoluteIndex);

					success = lua_rawequal(L, -1, -2);

					lua_pop(L, 2);
				}
				tracking.use(1);

				return success;
			}
		};

		template <>
		struct getter<Nz::Rectf>
		{
			static Nz::Rectf get(lua_State* L, int index, record& tracking) 
			{
				int absoluteIndex = lua_absindex(L, index);

				sol::stack_table rect(L, absoluteIndex);
				float x = rect["x"];
				float y = rect["y"];
				float width = rect["width"];
				float height = rect["height"];

				tracking.use(1);

				return Nz::Rectf(x, y, width, height);
			}
		};

		template <>
		struct getter<Nz::Vector2f>
		{
			static Nz::Vector2f get(lua_State* L, int index, record& tracking)
			{
				int absoluteIndex = lua_absindex(L, index);

				sol::table vec(L, absoluteIndex);
				float x = vec["x"];
				float y = vec["y"];

				tracking.use(1);

				return Nz::Vector2f(x, y);
			}
		};

		template <>
		struct pusher<bw::InputData>
		{
			static int push(lua_State* L, const bw::InputData& inputs)
			{
				lua_createtable(L, 0, 4);
				sol::stack_table vec(L);
				vec["isAttacking"] = inputs.isAttacking;
				vec["isJumping"] = inputs.isJumping;
				vec["isMovingLeft"] = inputs.isMovingLeft;
				vec["isMovingRight"] = inputs.isMovingRight;

				return 1;
			}
		};

		template <>
		struct pusher<Nz::Vector2f>
		{
			static int push(lua_State* L, const Nz::Vector2f& v)
			{
				lua_createtable(L, 0, 2);
				luaL_setmetatable(L, "vec2");
				sol::stack_table vec(L);
				vec["x"] = v.x;
				vec["y"] = v.y;

				return 1;
			}
		};
	}
}