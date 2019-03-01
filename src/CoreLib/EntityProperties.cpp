// Copyright (C) 2016 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in Prerequisites.hpp

#include <CoreLib/EntityProperties.hpp>
#include <CoreLib/Utils.hpp>

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

	EntityProperty TranslateEntityPropertyFromLua(const sol::object& value, PropertyType expectedType, bool isArray)
	{
		if (isArray)
		{
			sol::table content = value.as<sol::table>();
			std::size_t elementCount = content.size();
			if (elementCount == 0)
				throw std::runtime_error("Property array must contain at least one element");

			auto HandleDataArray = [&](auto dummyType) -> EntityProperty
			{
				using T = std::decay_t<decltype(dummyType)>;

				EntityPropertyArray<T> container(elementCount);
				for (std::size_t i = 0; i < elementCount; ++i)
					container[i] = content[i];

				return container;
			};

			switch (expectedType)
			{
				case PropertyType::Bool:
					return HandleDataArray(bool());
					
				case PropertyType::Float:
					return HandleDataArray(float());

				case PropertyType::Integer:
					return HandleDataArray(Nz::Int64());

				case PropertyType::String:
				case PropertyType::Texture:
					return HandleDataArray(std::string());

				default:
					break;
			}
		}
		else
		{
			switch (expectedType)
			{
				case PropertyType::Bool:
					return value.as<bool>();

				case PropertyType::Float:
					return value.as<float>();

				case PropertyType::Integer:
					return value.as<Nz::Int64>();

				case PropertyType::String:
				case PropertyType::Texture:
					return value.as<std::string>();

				default:
					break;
			}
		}

		throw std::runtime_error("Unhandled type");
	}

	sol::object TranslateEntityPropertyToLua(sol::state_view& lua, const EntityProperty& property)
	{
		return std::visit([&](auto&& value) -> sol::object
		{
			using T = std::decay_t<decltype(value)>;
			constexpr bool IsArray = IsSameTpl_v<EntityPropertyArray, T>;
			using PropertyType = std::conditional_t<IsArray, IsSameTpl<EntityPropertyArray, T>::ContainedType, T>;

			if constexpr (std::is_same_v<PropertyType, bool> ||
				std::is_same_v<PropertyType, float> ||
				std::is_same_v<PropertyType, Nz::Int64> ||
				std::is_same_v<PropertyType, std::string>)
			{
				if constexpr (IsArray)
				{
					std::size_t elementCount = value.GetSize();
					sol::table content = lua.create_table(int(elementCount));

					for (std::size_t i = 0; i < elementCount; ++i)
						content[i + 1] = sol::make_object(lua, value[i]);

					return content;
				}
				else
					return sol::make_object(lua, value);
			}
			else
				static_assert(AlwaysFalse<T>::value, "non-exhaustive visitor");

		}, property);
	}
}