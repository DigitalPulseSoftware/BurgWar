// Copyright (C) 2016 Jérôme Leclercq
// This file is part of the "Burgwar Common" project
// For conditions of distribution and use, see copyright notice in Prerequisites.hpp

#include <Common/EntityProperties.hpp>

namespace bw
{
	PropertyType ParsePropertyType(const std::string_view& str)
	{
		if (str == "bool" || str == "boolean")
			return PropertyType::Bool;
		else if (str == "float" || str == "double")
			return PropertyType::Float;
		else if (str == "int" || str == "integer")
			return PropertyType::Integer;
		else if (str == "string")
			return PropertyType::String;
		else if (str == "texture")
			return PropertyType::Texture;

		throw std::runtime_error("Invalid property type " + std::string(str));
	}
}