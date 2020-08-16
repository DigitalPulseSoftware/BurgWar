// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/Events.hpp>
#include <cassert>
#include <cctype>

namespace bw
{
	std::optional<ScriptingEvent> RetrieveScriptingEvent(const std::string_view& eventName)
	{
		auto CompareString = [](const std::string_view& lhs, const std::string_view& rhs) -> bool
		{
			if (lhs.size() != rhs.size())
				return false;

			for (std::size_t i = 0; i < lhs.size(); ++i)
			{
				if (std::tolower(lhs[i]) != std::tolower(rhs[i]))
					return false;
			}

			return true;
		};

		for (std::size_t i = 0; i < ScriptingEventCount; ++i)
		{
			ScriptingEvent event = static_cast<ScriptingEvent>(i);
			if (CompareString(ToString(event), eventName))
				return event;
		}

		return {};
	}

	std::string_view ToString(ScriptingEvent event)
	{
		static_assert(ScriptingEventCount == 12, "Please update the following switch");

		switch (event)
		{
			case ScriptingEvent::CollisionStart: return "CollisionStart";
			case ScriptingEvent::Death:          return "Death";
			case ScriptingEvent::Destroyed:      return "Destroyed";
			case ScriptingEvent::Died:           return "Died";
			case ScriptingEvent::HealthUpdate:   return "HealthUpdate";
			case ScriptingEvent::Init:           return "Init";
			case ScriptingEvent::Tick:           return "Tick";
			case ScriptingEvent::Frame:          return "Frame";
			case ScriptingEvent::PostFrame:      return "PostFrame";
			case ScriptingEvent::InputUpdate:    return "InputUpdate";
			case ScriptingEvent::Attack:         return "Attack";
			case ScriptingEvent::AttackFinish:   return "AttackFinish";
		}

		assert(!"Unhandled scripting event");
		return nullptr;
	}
}
