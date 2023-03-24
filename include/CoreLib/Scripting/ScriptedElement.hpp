// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SCRIPTING_SCRIPTEDELEMENT_HPP
#define BURGWAR_CORELIB_SCRIPTING_SCRIPTEDELEMENT_HPP

#include <CoreLib/PropertyValues.hpp>
#include <CoreLib/Scripting/ElementEvents.hpp>
#include <CoreLib/Scripting/ScriptedEvent.hpp>
#include <CoreLib/Scripting/ScriptedProperty.hpp>
#include <NazaraUtils/Prerequisites.hpp>
#include <tsl/hopscotch_map.h>
#include <sol/sol.hpp>
#include <array>
#include <memory>
#include <optional>
#include <string>

namespace bw
{
	struct ScriptedElement : std::enable_shared_from_this<ScriptedElement>
	{
		struct Callback
		{
			sol::main_protected_function callback;
			std::size_t callbackId;
			bool async = false;
		};

		sol::main_table elementTable;
		std::array<std::vector<Callback>, ElementEventCount> eventCallbacks;
		std::size_t nextCallbackId = 1;
		std::string base;
		std::string name;
		std::string fullName;
		std::vector<ScriptedEvent> customEvents;
		std::vector<std::vector<Callback>> customEventCallbacks;
		tsl::hopscotch_map<std::string /*key*/, ScriptedProperty> properties;
		tsl::hopscotch_map<std::string /*eventName*/, std::size_t> customEventByName;
	};
}

#endif
