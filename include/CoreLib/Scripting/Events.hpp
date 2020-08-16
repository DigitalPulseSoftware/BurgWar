// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SCRIPTING_EVENTS_HPP
#define BURGWAR_CORELIB_SCRIPTING_EVENTS_HPP

#include <CoreLib/Scripting/EventCombinator.hpp>
#include <cstddef>
#include <functional>
#include <optional>
#include <string_view>

namespace bw
{
	enum class ScriptingEvent
	{
		/*** Elements ***/

		// Shared element events
		CollisionStart,
		Death,
		Destroyed,
		Died,
		HealthUpdate,
		Init,
		Tick,

		// Client element events
		Frame,
		PostFrame,

		/*** Entities ***/

		InputUpdate,

		/*** Weapons ***/

		// Shared weapon events

		Attack,
		AttackFinish,

		Max = AttackFinish
	};

	template<ScriptingEvent E>
	struct ScriptingEventData
	{
		using ResultType = void;

		static constexpr bool FatalError = false;

	};

	template<>
	struct ScriptingEventData<ScriptingEvent::CollisionStart>
	{
		using ResultType = bool;

		static constexpr bool FatalError = false;
		static constexpr EventCombinator<bool, std::logical_and<bool>> Combinator = {};
	};

	template<>
	struct ScriptingEventData<ScriptingEvent::Init>
	{
		using ResultType = void;

		static constexpr bool FatalError = true;
	};

	constexpr std::size_t ScriptingEventCount = static_cast<std::size_t>(ScriptingEvent::Max) + 1;

	constexpr bool HasReturnValue(ScriptingEvent event);
	std::optional<ScriptingEvent> RetrieveScriptingEvent(const std::string_view& eventName);
	std::string_view ToString(ScriptingEvent event);
}

#include <CoreLib/Scripting/Events.inl>

#endif
