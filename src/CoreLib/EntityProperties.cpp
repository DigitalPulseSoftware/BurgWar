// Copyright (C) 2016 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in Prerequisites.hpp

#include <CoreLib/EntityProperties.hpp>
#include <CoreLib/Scripting/AbstractScriptingLibrary.hpp>
#include <CoreLib/Utils.hpp>

namespace bw
{
	std::pair<PropertyInternalType, bool> ExtractPropertyType(const EntityProperty& property)
	{
		// Use switch on index() instead of visitor for speed
		switch (property.index())
		{
			static_assert(std::is_same_v<std::variant_alternative_t<0, EntityProperty>, bool>);
			static_assert(std::is_same_v<std::variant_alternative_t<1, EntityProperty>, EntityPropertyArray<bool>>);
			
			case 0:
				return std::make_pair(PropertyInternalType::Bool, false);
			case 1:
				return std::make_pair(PropertyInternalType::Bool, true);

			static_assert(std::is_same_v<std::variant_alternative_t<2, EntityProperty>, float>);
			static_assert(std::is_same_v<std::variant_alternative_t<3, EntityProperty>, EntityPropertyArray<float>>);

			case 2:
				return std::make_pair(PropertyInternalType::Float, false);
			case 3:
				return std::make_pair(PropertyInternalType::Float, true);

			static_assert(std::is_same_v<std::variant_alternative_t<4, EntityProperty>, Nz::Int64>);
			static_assert(std::is_same_v<std::variant_alternative_t<5, EntityProperty>, EntityPropertyArray<Nz::Int64>>);

			case 4:
				return std::make_pair(PropertyInternalType::Integer, false);
			case 5:
				return std::make_pair(PropertyInternalType::Integer, true);

			static_assert(std::is_same_v<std::variant_alternative_t<6, EntityProperty>, Nz::Vector2f>);
			static_assert(std::is_same_v<std::variant_alternative_t<7, EntityProperty>, EntityPropertyArray<Nz::Vector2f>>);

			case 6:
				return std::make_pair(PropertyInternalType::Float2, false);
			case 7:
				return std::make_pair(PropertyInternalType::Float2, true);

			static_assert(std::is_same_v<std::variant_alternative_t<8, EntityProperty>, Nz::Vector2i64>);
			static_assert(std::is_same_v<std::variant_alternative_t<9, EntityProperty>, EntityPropertyArray<Nz::Vector2i64>>);

			case 8:
				return std::make_pair(PropertyInternalType::Integer2, false);
			case 9:
				return std::make_pair(PropertyInternalType::Integer2, true);

			static_assert(std::is_same_v<std::variant_alternative_t<10, EntityProperty>, std::string>);
			static_assert(std::is_same_v<std::variant_alternative_t<11, EntityProperty>, EntityPropertyArray<std::string>>);

			case 10:
				return std::make_pair(PropertyInternalType::String, false);
			case 11:
				return std::make_pair(PropertyInternalType::String, true);
		}

		throw std::runtime_error("Unhandled internal type");
	}

	PropertyType ParsePropertyType(const std::string_view& str)
	{
		if (str == "bool" || str == "boolean")
			return PropertyType::Bool;
		else if (str == "float" || str == "double")
			return PropertyType::Float;
		else if (str == "floatpos" || str == "floatposition")
			return PropertyType::FloatPosition;
		else if (str == "floatsize")
			return PropertyType::FloatSize;
		else if (str == "int" || str == "integer")
			return PropertyType::Integer;
		else if (str == "intpos" || str == "integerposition")
			return PropertyType::IntegerPosition;
		else if (str == "intsize" || str == "integersize")
			return PropertyType::IntegerSize;
		else if (str == "string")
			return PropertyType::String;
		else if (str == "texture")
			return PropertyType::Texture;

		throw std::runtime_error("Invalid property type " + std::string(str));
	}

	PropertyInternalType ParsePropertyInternalType(const std::string_view & str)
	{
		if (str == "bool")
			return PropertyInternalType::Bool;
		else if (str == "float")
			return PropertyInternalType::Float;
		else if (str == "float2")
			return PropertyInternalType::Float2;
		else if (str == "integer")
			return PropertyInternalType::Integer;
		else if (str == "integer2")
			return PropertyInternalType::Integer2;
		else if (str == "string")
			return PropertyInternalType::String;

		throw std::runtime_error("Invalid property type " + std::string(str));
	}

	const char* ToString(PropertyType propertyType)
	{
		switch (propertyType)
		{
			case PropertyType::Bool:
				return "bool";

			case PropertyType::Float:
				return "float";

			case PropertyType::FloatPosition:
				return "floatposition";

			case PropertyType::FloatSize:
				return "floatsize";

			case PropertyType::Integer:
				return "integer";

			case PropertyType::IntegerPosition:
				return "integerposition";

			case PropertyType::IntegerSize:
				return "integersize";

			case PropertyType::String:
				return "string";

			case PropertyType::Texture:
				return "texture";
		}

		assert(!"Unhandled property type");
		return nullptr;
	}

	const char* ToString(PropertyInternalType propertyType)
	{
		switch (propertyType)
		{
			case PropertyInternalType::Bool:
				return "bool";

			case PropertyInternalType::Float:
				return "float";

			case PropertyInternalType::Float2:
				return "float2";

			case PropertyInternalType::Integer:
				return "integer";

			case PropertyInternalType::Integer2:
				return "integer2";

			case PropertyInternalType::String:
				return "string";

			default:
				break;
		}

		assert(!"Unhandled property internal type");
		return nullptr;
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
					container[i] = content[i + 1];

				return container;
			};

			switch (expectedType)
			{
				case PropertyType::Bool:
					return HandleDataArray(bool());
					
				case PropertyType::Float:
					return HandleDataArray(float());

				case PropertyType::FloatPosition:
				case PropertyType::FloatSize:
					return HandleDataArray(Nz::Vector2f());

				case PropertyType::Integer:
					return HandleDataArray(Nz::Int64());

				case PropertyType::IntegerPosition:
				case PropertyType::IntegerSize:
					return HandleDataArray(Nz::Vector2i64());

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

				case PropertyType::FloatPosition:
				case PropertyType::FloatSize:
					return value.as<Nz::Vector2f>();

				case PropertyType::Integer:
					return value.as<Nz::Int64>();

				case PropertyType::IntegerPosition:
				case PropertyType::IntegerSize:
					return value.as<Nz::Vector2i64>();

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
			              std::is_same_v<PropertyType, Nz::Vector2f> ||
			              std::is_same_v<PropertyType, Nz::Vector2i64> ||
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