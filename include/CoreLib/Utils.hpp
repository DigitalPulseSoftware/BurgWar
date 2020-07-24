// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_UTILS_HPP
#define BURGWAR_CORELIB_UTILS_HPP

#include <Nazara/Math/Angle.hpp>
#include <Nazara/Math/Quaternion.hpp>
#include <Nazara/Math/Vector2.hpp>
#include <Nazara/Math/Vector3.hpp>
#include <string>
#include <type_traits>

namespace bw
{
	template<typename T>
	struct AlwaysFalse : std::false_type {};

	template<typename... Args>
	struct OverloadResolver
	{
		template<typename R, typename T>
		constexpr auto operator()(R(T::*ptr)(Args...)) const noexcept
		{
			return ptr;
		}

		template<typename R, typename T>
		constexpr auto operator()(R(T::*ptr)(Args...) const) const noexcept
		{
			return ptr;
		}

		template<typename R>
		constexpr auto operator()(R(*ptr)(Args...)) const noexcept
		{
			return ptr;
		}
	};

	template<typename... Args> constexpr OverloadResolver<Args...> Overload = {};

	// std::is_same but for containers
	template<template<typename...> typename T, typename U>
	struct IsSameTpl
	{
		using ContainedType = void;

		static constexpr bool value = false;
	};

	template<template<typename...> typename T, template<typename...> typename U, typename Us>
	struct IsSameTpl<T, U<Us>>
	{
		using ContainedType = Us;

		static constexpr bool value = std::is_same_v<T<Us>, U<Us>>;
	};

	template<template<typename...> typename T, typename U>
	inline constexpr bool IsSameTpl_v = IsSameTpl<T, U>::value;

	inline Nz::RadianAnglef AngleFromQuaternion(const Nz::Quaternionf& quat);
	template<typename T> Nz::Vector2<T> AlignPosition(Nz::Vector2<T> position, const Nz::Vector2<T>& alignment);
	template<Nz::AngleUnit Unit, typename T> bool CompareWithEpsilon(const Nz::Angle<Unit, T>& left, const Nz::Angle<Unit, T>& right, T epsilon);
	template<typename T> bool CompareWithEpsilon(const Nz::Vector2<T>& left, const Nz::Vector2<T>& right, T epsilon);
	std::string ByteToString(Nz::UInt64 bytes, bool speed = false);
	Nz::Vector3f DampenedString(const Nz::Vector3f& currentPos, const Nz::Vector3f& targetPos, float frametime, float springStrength = 3.f);
	template<typename T> bool IsMoreRecent(T a, T b);
	inline std::string ReplaceStr(std::string str, const std::string_view& from, const std::string_view& to);
	template<typename F> bool SplitString(const std::string_view& str, const std::string_view& token, F&& func);
	template<typename F> bool SplitStringAny(const std::string_view& str, const std::string_view& token, F&& func);

	template<typename E> auto UnderlyingCast(E value) -> std::underlying_type_t<E>;
}

#include <CoreLib/Utils.inl>

#endif
