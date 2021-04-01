// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SCRIPTING_SCRIPTEDPROPERTY_HPP
#define BURGWAR_CORELIB_SCRIPTING_SCRIPTEDPROPERTY_HPP

#include <CoreLib/PropertyValues.hpp>

namespace bw
{
	struct ScriptedProperty
	{
		PropertyType type;
		std::optional<PropertyValue> defaultValue;
		std::size_t index;
		bool isArray = false;
		bool shared = false;
	};

	BURGWAR_CORELIB_API ScriptedProperty InitPropertyFromLua(std::size_t index, const sol::table& table);
}

#endif
