// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SCRIPTING_ELEMENTEVENTS_HPP
#define BURGWAR_CORELIB_SCRIPTING_ELEMENTEVENTS_HPP

#include <Nazara/Prerequisites.hpp>
#include <CoreLib/Scripting/EventCombinator.hpp>
#include <cstddef>
#include <functional>
#include <optional>
#include <string_view>

namespace bw
{
	enum class ElementEvent
	{
#define BURGWAR_EVENT(X) X,
#define BURGWAR_EVENT_LAST(X) X, Max = X

#include <CoreLib/Scripting/ElementEventList.hpp>
	};

	template<ElementEvent E>
	struct ElementEventData
	{
		using ResultType = void;

		static constexpr bool FatalError = false;
	};

	template<>
	struct ElementEventData<ElementEvent::CollisionStart>
	{
		using ResultType = bool;

		static constexpr bool FatalError = false;
		static constexpr EventCombinator<bool, std::logical_and<bool>> Combinator = {};
	};

	template<>
	struct ElementEventData<ElementEvent::Init>
	{
		using ResultType = void;

		static constexpr bool FatalError = true;
	};

	template<>
	struct ElementEventData<ElementEvent::TakeDamage>
	{
		using ResultType = Nz::UInt16;

		static constexpr bool FatalError = false;
		static constexpr EventCombinator<Nz::UInt16, std::minus<Nz::UInt16>> Combinator = {};
	};

	constexpr std::size_t ElementEventCount = static_cast<std::size_t>(ElementEvent::Max) + 1;

	constexpr bool HasReturnValue(ElementEvent event);
	std::optional<ElementEvent> RetrieveElementEvent(const std::string_view& eventName);
	std::string_view ToString(ElementEvent event);
}

#include <CoreLib/Scripting/ElementEvents.inl>

#endif
