// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/AbstractScriptingLibrary.hpp>
#include <Nazara/Math/Angle.hpp>
#include <Nazara/Math/Rect.hpp>
#include <Nazara/Math/Vector2.hpp>
#include <CoreLib/InputData.hpp>
#include <sol2/sol.hpp>
#include <cassert>

namespace bw
{
}

namespace sol
{
	template<>
	struct lua_size<bw::InputData> : std::integral_constant<int, 1> {};

	template<>
	struct lua_size<Nz::DegreeAnglef> : std::integral_constant<int, 1> {};

	template<>
	struct lua_size<Nz::Rectf> : std::integral_constant<int, 1> {};

	template<typename T>
	struct lua_size<Nz::Vector2<T>> : std::integral_constant<int, 1> {};

	template<>
	struct lua_type_of<Nz::DegreeAnglef> : std::integral_constant<sol::type, sol::type::number> {};

	template<>
	struct lua_type_of<Nz::DegreeAnglef> : std::integral_constant<sol::type, sol::type::number> {};

	template<>
	struct lua_type_of<bw::InputData> : std::integral_constant<sol::type, sol::type::table> {};

	template<>
	struct lua_type_of<Nz::Rectf> : std::integral_constant<sol::type, sol::type::table> {};

	template<typename T>
	struct lua_type_of<Nz::Vector2<T>> : std::integral_constant<sol::type, sol::type::table> {};

	namespace stack
	{
		template<typename T>
		struct checker<Nz::Vector2<T>>
		{
			static_assert(std::is_arithmetic_v<T>);

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
		struct getter<Nz::DegreeAnglef>
		{
			static Nz::DegreeAnglef get(lua_State* L, int index, record& tracking)
			{
				int absoluteIndex = lua_absindex(L, index);

				sol::stack_object obj(L, absoluteIndex);
				return Nz::DegreeAnglef(obj.as<float>());
			}
		};

		template <>
		struct getter<Nz::DegreeAnglef>
		{
			static Nz::DegreeAnglef get(lua_State* L, int index, record& tracking)
			{
				int absoluteIndex = lua_absindex(L, index);

				sol::stack_object obj(L, absoluteIndex);
				return Nz::DegreeAnglef(obj.as<float>());
			}
		};

		template <>
		struct getter<Nz::Rectf>
		{
			static Nz::Rectf get(lua_State* L, int index, record& tracking) 
			{
				int absoluteIndex = lua_absindex(L, index);

				sol::table rect(L, absoluteIndex);
				float x = rect["x"];
				float y = rect["y"];
				float width = rect["width"];
				float height = rect["height"];

				tracking.use(1);

				return Nz::Rectf(x, y, width, height);
			}
		};

		template<typename T>
		struct getter<Nz::Vector2<T>>
		{
			static_assert(std::is_arithmetic_v<T>);

			static Nz::Vector2<T> get(lua_State* L, int index, record& tracking)
			{
				int absoluteIndex = lua_absindex(L, index);

				sol::table vec(L, absoluteIndex);
				T x = vec["x"];
				T y = vec["y"];

				tracking.use(1);

				return Nz::Vector2<T>(x, y);
			}
		};

		template <>
		struct pusher<bw::InputData>
		{
			static int push(lua_State* L, const bw::InputData& inputs)
			{
				lua_createtable(L, 0, 4);
				sol::stack_table vec(L);
				vec["aimDirection"] = inputs.aimDirection;
				vec["isAttacking"] = inputs.isAttacking;
				vec["isJumping"] = inputs.isJumping;
				vec["isMovingLeft"] = inputs.isMovingLeft;
				vec["isMovingRight"] = inputs.isMovingRight;

				return 1;
			}
		};

		template <>
		struct pusher<Nz::DegreeAnglef>
		{
			static int push(lua_State* L, const Nz::DegreeAnglef& degreeAngle)
			{
				lua_pushnumber(L, degreeAngle.ToDegrees());

				return 1;
			}
		};

		template<typename T>
		struct pusher<Nz::Vector2<T>>
		{
			static_assert(std::is_arithmetic_v<T>);

			static int push(lua_State* L, const Nz::Vector2<T>& v)
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