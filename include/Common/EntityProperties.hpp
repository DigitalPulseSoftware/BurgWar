// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar Common" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_COMMON_ENTITYPROPERTIES_HPP
#define BURGWAR_COMMON_ENTITYPROPERTIES_HPP

#include <Nazara/Prerequisites.hpp>
#include <hopscotch/hopscotch_map.h>
#include <variant>

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

	PropertyType ParsePropertyType(const std::string_view& str);

	using EntityProperty = std::variant<std::monostate, bool, float, Nz::Int64, std::string>;
	using EntityProperties = tsl::hopscotch_map<std::string /*propertyName*/, EntityProperty /*property*/>;
}

#endif
