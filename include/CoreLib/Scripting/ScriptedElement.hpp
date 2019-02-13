// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SCRIPTING_SCRIPTEDELEMENT_HPP
#define BURGWAR_CORELIB_SCRIPTING_SCRIPTEDELEMENT_HPP

#include <CoreLib/EntityProperties.hpp>
#include <Nazara/Prerequisites.hpp>
#include <hopscotch/hopscotch_map.h>
#include <sol2/sol.hpp>
#include <memory>
#include <string>

namespace bw
{
	struct ScriptedElement : std::enable_shared_from_this<ScriptedElement>
	{
		struct Property
		{
			PropertyType type;
			EntityProperty defaultValue;
			bool shared = false;
		};

		sol::table elementTable;
		sol::reference tickFunction;
		std::string name;
		std::string fullName;
		tsl::hopscotch_map<std::string /*key*/, Property> properties;
	};
}

#endif
