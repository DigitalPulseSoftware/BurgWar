// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_SHARED_SCRIPTING_SCRIPTEDELEMENT_HPP
#define BURGWAR_SHARED_SCRIPTING_SCRIPTEDELEMENT_HPP

#include <hopscotch/hopscotch_map.h>
#include <sol2/sol.hpp>
#include <memory>
#include <string>

namespace bw
{
	enum class PropertyType
	{
		Bool,
		Float,
		Integer,
		String,
		Texture
	};

	using EntityProperty = std::variant<std::monostate, bool, float, int, std::string>;

	struct ScriptedElement : std::enable_shared_from_this<ScriptedElement>
	{
		struct Property
		{
			PropertyType type;
			EntityProperty defaultValue;
		};

		sol::table elementTable;
		sol::reference tickFunction;
		std::string name;
		std::string fullName;
		tsl::hopscotch_map<std::string /*key*/, Property> properties;
	};
}

#endif
