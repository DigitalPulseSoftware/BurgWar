// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/AbstractScriptingLibrary.hpp>
#include <Nazara/Core/Color.hpp>
#include <Nazara/Core/ObjectHandle.hpp>
#include <Nazara/Math/Angle.hpp>
#include <Nazara/Math/Rect.hpp>
#include <Nazara/Math/Vector2.hpp>
#include <Nazara/Math/Vector3.hpp>
#include <NDK/Entity.hpp>
#include <CoreLib/PlayerInputData.hpp>
#include <Thirdparty/sol3/sol.hpp>
#include <cassert>

namespace bw
{
	class Player;

	using PlayerHandle = Nz::ObjectHandle<Player>;

	inline AbstractScriptingLibrary::AbstractScriptingLibrary(const Logger& logger) :
	m_logger(logger)
	{
	}

	inline const Logger& AbstractScriptingLibrary::GetLogger() const
	{
		return m_logger;
	}
}

namespace sol
{
	template <>
	struct is_automagical<Ndk::EntityHandle> : std::false_type {};

	template <>
	struct is_automagical<bw::PlayerHandle> : std::false_type {};

	template<>
	struct lua_size<bw::PlayerInputData> : std::integral_constant<int, 1> {};

	template<>
	struct lua_size<Nz::Color> : std::integral_constant<int, 1> {};

	template<>
	struct lua_size<Nz::DegreeAnglef> : std::integral_constant<int, 1> {};

	template<typename T>
	struct lua_size<Nz::Rect<T>> : std::integral_constant<int, 1> {};

	template<typename T>
	struct lua_size<Nz::Vector2<T>> : std::integral_constant<int, 1> {};

	template<typename T>
	struct lua_size<Nz::Vector3<T>> : std::integral_constant<int, 1> {};

	template<>
	struct lua_type_of<Nz::Color> : std::integral_constant<sol::type, sol::type::table> {};

	template<>
	struct lua_type_of<Nz::DegreeAnglef> : std::integral_constant<sol::type, sol::type::number> {};

	template<>
	struct lua_type_of<bw::PlayerInputData> : std::integral_constant<sol::type, sol::type::table> {};

	template<>
	struct lua_type_of<Nz::RadianAnglef> : std::integral_constant<sol::type, sol::type::number> {};

	template<typename T>
	struct lua_type_of<Nz::Rect<T>> : std::integral_constant<sol::type, sol::type::table> {};

	template<typename T>
	struct lua_type_of<Nz::Vector2<T>> : std::integral_constant<sol::type, sol::type::table> {};

	template<typename T>
	struct lua_type_of<Nz::Vector3<T>> : std::integral_constant<sol::type, sol::type::table> {};

	template <typename T>
	struct unique_usertype_traits<Nz::ObjectHandle<T>>
	{
		using type = T;
		using actual_type = Nz::ObjectHandle<T>;
		static const bool value = true;

		static bool is_null(const actual_type& ptr) {
			return !ptr.IsValid();
		}

		static type* get(const actual_type& ptr) {
			return ptr.GetObject();
		}
	};

	template <typename T, typename Handler>
	bool sol_lua_check(sol::types<Nz::Vector2<T>>, lua_State* L, int index, Handler&& handler, sol::stack::record& tracking)
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
	
	template <typename T, typename Handler>
	bool sol_lua_check(sol::types<Nz::Vector3<T>>, lua_State* L, int index, Handler&& handler, sol::stack::record& tracking)
	{
		int absoluteIndex = lua_absindex(L, index);
		bool success = stack::check<sol::table>(L, absoluteIndex, handler);
		if (success)
		{
			luaL_getmetatable(L, "vec3");
			sol::stack_table expectedMetatable;

			lua_getmetatable(L, absoluteIndex);

			success = lua_rawequal(L, -1, -2);

			lua_pop(L, 2);
		}
		tracking.use(1);

		return success;
	}

	inline Nz::Color sol_lua_get(sol::types<Nz::Color>, lua_State* L, int index, sol::stack::record& tracking)
	{
		int absoluteIndex = lua_absindex(L, index);

		sol::table colorTable = sol::stack::get<sol::table>(L, absoluteIndex);
		Nz::UInt8 r = colorTable["r"];
		Nz::UInt8 g = colorTable["g"];
		Nz::UInt8 b = colorTable["b"];
		Nz::UInt8 a = colorTable.get_or("a", Nz::UInt8(255));

		tracking.use(1);

		return Nz::Color(r, g, b, a);
	}

	inline Nz::DegreeAnglef sol_lua_get(sol::types<Nz::DegreeAnglef>, lua_State* L, int index, sol::stack::record& tracking)
	{
		int absoluteIndex = lua_absindex(L, index);

		float angle = sol::stack::get<float>(L, absoluteIndex);
		tracking.use(1);

		return Nz::DegreeAnglef(angle);
	}

	inline Nz::RadianAnglef sol_lua_get(sol::types<Nz::RadianAnglef>, lua_State* L, int index, sol::stack::record& tracking)
	{
		int absoluteIndex = lua_absindex(L, index);

		float angle = sol::stack::get<float>(L, absoluteIndex);
		tracking.use(1);

		return Nz::DegreeAnglef(angle);
	}

	template<typename T>
	inline Nz::Rect<T> sol_lua_get(sol::types<Nz::Rect<T>>, lua_State* L, int index, sol::stack::record& tracking)
	{
		int absoluteIndex = lua_absindex(L, index);

		sol::table rect = sol::stack::get<sol::table>(L, absoluteIndex);
		T x = rect["x"];
		T y = rect["y"];
		T width = rect["width"];
		T height = rect["height"];

		tracking.use(1);

		return Nz::Rect<T>(x, y, width, height);
	}

	template<typename T>
	Nz::Vector2<T> sol_lua_get(sol::types<Nz::Vector2<T>>, lua_State* L, int index, sol::stack::record& tracking)
	{
		int absoluteIndex = lua_absindex(L, index);

		sol::table rect = sol::stack::get<sol::table>(L, absoluteIndex);
		T x = rect["x"];
		T y = rect["y"];

		tracking.use(1);

		return Nz::Vector2<T>(x, y);
	}

	template<typename T>
	Nz::Vector3<T> sol_lua_get(sol::types<Nz::Vector3<T>>, lua_State* L, int index, sol::stack::record& tracking)
	{
		int absoluteIndex = lua_absindex(L, index);

		sol::table rect = sol::stack::get<sol::table>(L, absoluteIndex);
		T x = rect["x"];
		T y = rect["y"];
		T z = rect["z"];

		tracking.use(1);

		return Nz::Vector3<T>(x, y, z);
	}

	inline int sol_lua_push(sol::types<bw::PlayerInputData>, lua_State* L, const bw::PlayerInputData& inputs)
	{
		lua_createtable(L, 0, 6);
		sol::stack_table vec(L);
		vec["aimDirection"] = inputs.aimDirection;
		vec["isAttacking"] = inputs.isAttacking;
		vec["isCrouching"] = inputs.isCrouching;
		vec["isLookingRight"] = inputs.isLookingRight;
		vec["isJumping"] = inputs.isJumping;
		vec["isMovingLeft"] = inputs.isMovingLeft;
		vec["isMovingRight"] = inputs.isMovingRight;

		return 1;
	}

	template<typename T>
	int sol_lua_push(sol::types<Nz::DegreeAngle<T>>, lua_State* L, const Nz::DegreeAngle<T>& angle)
	{
		return sol::stack::push(L, angle.ToDegrees());
	}

	template<typename T>
	int sol_lua_push(sol::types<Nz::RadianAngle<T>>, lua_State* L, const Nz::RadianAngle<T>& angle)
	{
		return sol::stack::push(L, angle.ToDegrees());
	}

	template<typename T>
	int sol_lua_push(sol::types<Nz::Rect<T>>, lua_State* L, const Nz::Rect<T>& rect)
	{
		lua_createtable(L, 0, 4);
		luaL_setmetatable(L, "rect");
		sol::stack_table vec(L);
		vec["x"] = rect.x;
		vec["y"] = rect.y;
		vec["width"] = rect.width;
		vec["height"] = rect.height;

		return 1;
	}

	template<typename T>
	int sol_lua_push(sol::types<Nz::Vector2<T>>, lua_State* L, const Nz::Vector2<T>& v)
	{
		lua_createtable(L, 0, 2);
		luaL_setmetatable(L, "vec2");
		sol::stack_table vec(L);
		vec["x"] = v.x;
		vec["y"] = v.y;

		return 1;
	}

	template<typename T>
	int sol_lua_push(sol::types<Nz::Vector3<T>>, lua_State* L, const Nz::Vector3<T>& v)
	{
		lua_createtable(L, 0, 3);
		luaL_setmetatable(L, "vec3");
		sol::stack_table vec(L);
		vec["x"] = v.x;
		vec["y"] = v.y;
		vec["z"] = v.z;

		return 1;
	}
}
