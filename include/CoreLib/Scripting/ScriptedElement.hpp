// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SCRIPTING_SCRIPTEDELEMENT_HPP
#define BURGWAR_CORELIB_SCRIPTING_SCRIPTEDELEMENT_HPP

#include <CoreLib/EntityProperties.hpp>
#include <CoreLib/Scripting/Events.hpp>
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
		struct Property
		{
			PropertyType type;
			std::optional<EntityProperty> defaultValue;
			std::size_t index;
			bool isArray = false;
			bool shared = false;
		};

		sol::table elementTable;
		std::array<std::vector<sol::protected_function>, ScriptingEventCount> events;
		std::string base;
		std::string name;
		std::string fullName;
		tsl::hopscotch_map<std::string /*key*/, Property> properties;
	};
}

#endif
