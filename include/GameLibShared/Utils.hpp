// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_SHARED_UTILS_HPP
#define BURGWAR_SHARED_UTILS_HPP

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

	template<typename... Args> constexpr OverloadResolver<Args...> Overload = {};

	Nz::Vector3f DampenedString(const Nz::Vector3f& currentPos, const Nz::Vector3f& targetPos, float frametime, float springStrength = 3.f);
}

#endif
