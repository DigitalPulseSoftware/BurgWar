// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SCRIPTING_EVENTS_HPP
#define BURGWAR_CORELIB_SCRIPTING_EVENTS_HPP

#include <cstddef>
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

	constexpr std::size_t ScriptingEventCount = static_cast<std::size_t>(ScriptingEvent::Max) + 1;

	std::optional<ScriptingEvent> RetrieveScriptingEvent(const std::string_view& eventName);
	std::string_view ToString(ScriptingEvent event);
}

#endif
