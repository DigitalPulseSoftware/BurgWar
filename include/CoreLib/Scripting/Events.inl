// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/Events.hpp>
#include <cassert>

namespace bw
{
	constexpr bool HasReturnValue(ScriptingEvent event)
	{
		switch (event)
		{
			case ScriptingEvent::CollisionStart: return !std::is_same_v<ScriptingEventData<ScriptingEvent::CollisionStart>::ResultType, void>;
			case ScriptingEvent::Death:          return !std::is_same_v<ScriptingEventData<ScriptingEvent::Death>::ResultType, void>;
			case ScriptingEvent::Destroyed:      return !std::is_same_v<ScriptingEventData<ScriptingEvent::Destroyed>::ResultType, void>;
			case ScriptingEvent::Died:           return !std::is_same_v<ScriptingEventData<ScriptingEvent::Died>::ResultType, void>;
			case ScriptingEvent::HealthUpdate:   return !std::is_same_v<ScriptingEventData<ScriptingEvent::HealthUpdate>::ResultType, void>;
			case ScriptingEvent::Init:           return !std::is_same_v<ScriptingEventData<ScriptingEvent::Init>::ResultType, void>;
			case ScriptingEvent::Tick:           return !std::is_same_v<ScriptingEventData<ScriptingEvent::Tick>::ResultType, void>;
			case ScriptingEvent::Frame:          return !std::is_same_v<ScriptingEventData<ScriptingEvent::Frame>::ResultType, void>;
			case ScriptingEvent::PostFrame:      return !std::is_same_v<ScriptingEventData<ScriptingEvent::PostFrame>::ResultType, void>;
			case ScriptingEvent::InputUpdate:    return !std::is_same_v<ScriptingEventData<ScriptingEvent::InputUpdate>::ResultType, void>;
			case ScriptingEvent::Attack:         return !std::is_same_v<ScriptingEventData<ScriptingEvent::Attack>::ResultType, void>;
			case ScriptingEvent::AttackFinish:   return !std::is_same_v<ScriptingEventData<ScriptingEvent::AttackFinish>::ResultType, void>;
		}

		return false;
	}
}
