// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/ScriptedProperty.hpp>

namespace bw
{
	ScriptedProperty InitPropertyFromLua(std::size_t index, const sol::table& table)
	{
		ScriptedProperty property;
		property.index = index;
		property.type = table["Type"];

		sol::object propertyShared = table["Shared"];
		if (propertyShared)
			property.shared = propertyShared.as<bool>();

		sol::object propertyArray = table["Array"];
		if (propertyArray)
			property.isArray = propertyArray.as<bool>();

		sol::object propertyDefault = table["Default"];
		if (!propertyDefault.is<sol::nil_t>())
			property.defaultValue = TranslatePropertyFromLua(nullptr, propertyDefault, property.type, property.isArray);

		return property;
	}
}
