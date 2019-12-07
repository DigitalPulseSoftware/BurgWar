// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_UTILS_HPP
#define BURGWAR_CORELIB_UTILS_HPP

#include <Nazara/Math/Quaternion.hpp>
#include <Nazara/Math/Vector3.hpp>
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

	template<typename... Args> constexpr OverloadResolver<Args...> Overload = {};

	inline Nz::RadianAnglef AngleFromQuaternion(const Nz::Quaternionf& quat);
	Nz::Vector3f DampenedString(const Nz::Vector3f& currentPos, const Nz::Vector3f& targetPos, float frametime, float springStrength = 3.f);
	template<typename T> bool IsMoreRecent(T a, T b);
}

#include <CoreLib/Utils.inl>

#endif
