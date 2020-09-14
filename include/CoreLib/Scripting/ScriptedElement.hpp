// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SCRIPTING_SCRIPTEDELEMENT_HPP
#define BURGWAR_CORELIB_SCRIPTING_SCRIPTEDELEMENT_HPP

#include <CoreLib/PropertyValues.hpp>
#include <CoreLib/Scripting/ElementEvents.hpp>
#include <CoreLib/Scripting/ScriptedProperty.hpp>
#include <Nazara/Prerequisites.hpp>
#include <Thirdparty/tsl/hopscotch_map.h>
#include <Thirdparty/sol3/sol.hpp>
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
			bool async = false;
		};

		std::array<std::vector<Callback>, ElementEventCount> events;
		sol::main_table elementTable;
		std::string base;
		std::string name;
		std::string fullName;
		tsl::hopscotch_map<std::string /*key*/, ScriptedProperty> properties;
	};
}

#endif
