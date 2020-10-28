// Copyright (C) 2016 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in Prerequisites.hpp

#include <CoreLib/PropertyValues.hpp>
#include <CoreLib/SharedMatch.hpp>
#include <CoreLib/Components/ScriptComponent.hpp>
#include <CoreLib/Scripting/AbstractElementLibrary.hpp>
#include <CoreLib/Scripting/AbstractScriptingLibrary.hpp>
#include <CoreLib/Utils.hpp>

namespace bw
{
	namespace
	{
		template<bool IsArray, typename T, typename ConvertFunc>
		sol::object PushValue(sol::state_view& lua, T&& value, ConvertFunc&& convertFunc)
		{
			if constexpr (IsArray)
			{
				std::size_t elementCount = value.GetSize();
				sol::table content = lua.create_table(int(elementCount));

				for (std::size_t i = 0; i < elementCount; ++i)
					content[i + 1] = convertFunc(value[i]);

				return content;
			}
			else
				return convertFunc(*value);
		};

		template<bool IsArray, typename T>
		sol::object PushValuePassthrough(sol::state_view& lua, T&& value)
		{
			// I had troubles using a passthrough (std::forward) function with PushValue
			/*constexpr bool IsArray = IsSameTpl_v<PropertyArray, T>;
			using InternalType = std::conditional_t<IsArray, typename IsSameTpl<PropertyArray, T>::ContainedType, T>;

			return PushValue(lua, value, [&lua](InternalType&& val) -> sol::object
			{
				return sol::make_object(lua, std::forward<InternalType>(val));
			});*/

			if constexpr (IsArray)
			{
				std::size_t elementCount = value.GetSize();
				sol::table content = lua.create_table(int(elementCount));

				for (std::size_t i = 0; i < elementCount; ++i)
					content[i + 1] = sol::make_object(lua, value[i]);

				return content;
			}
			else
				return sol::make_object(lua, *value);
		};
	}

	std::pair<PropertyType, bool> ExtractPropertyType(const PropertyValue& value)
	{
		return std::visit([&](auto&& arg)
		{
			using T = std::decay_t<decltype(arg)>;
			using TypeExtractor = PropertyTypeExtractor<T>;

			return std::make_pair(TypeExtractor::Property, TypeExtractor::IsArray);
		}, value);
	}

	PropertyType ParsePropertyType(const std::string_view& str)
	{
		if (str == "bool" || str == "boolean")
			return PropertyType::Bool;
		else if (str == "entity")
			return PropertyType::Entity;
		else if (str == "float" || str == "double")
			return PropertyType::Float;
		else if (str == "floatpos" || str == "floatposition")
			return PropertyType::FloatPosition;
		else if (str == "floatposition3d")
			return PropertyType::FloatPosition3D;
		else if (str == "floatrect")
			return PropertyType::FloatRect;
		else if (str == "floatsize")
			return PropertyType::FloatSize;
		else if (str == "floatsize3d")
			return PropertyType::FloatSize3D;
		else if (str == "int" || str == "integer")
			return PropertyType::Integer;
		else if (str == "intpos" || str == "integerposition")
			return PropertyType::IntegerPosition;
		else if (str == "intpos3" || str == "integerposition3" || str == "intpos3d" || str == "integerposition3d")
			return PropertyType::IntegerPosition3D;
		else if (str == "intrect")
			return PropertyType::IntegerRect;
		else if (str == "intsize" || str == "integersize")
			return PropertyType::IntegerSize;
		else if (str == "intsize3d" || str == "integersize3d")
			return PropertyType::IntegerSize3D;
		else if (str == "layer")
			return PropertyType::Layer;
		else if (str == "string")
			return PropertyType::String;
		else if (str == "texture")
			return PropertyType::Texture;

		throw std::runtime_error("Invalid property type " + std::string(str));
	}

	const char* ToString(PropertyType propertyType)
	{
		switch (propertyType)
		{
			case PropertyType::Bool:
				return "bool";

			case PropertyType::Entity:
				return "entity";

			case PropertyType::Float:
				return "float";

			case PropertyType::FloatPosition:
				return "floatposition";

			case PropertyType::FloatPosition3D:
				return "floatposition3d";

			case PropertyType::FloatRect:
				return "floatrect";

			case PropertyType::FloatSize:
				return "floatsize";

			case PropertyType::FloatSize3D:
				return "floatsize3d";

			case PropertyType::Integer:
				return "integer";

			case PropertyType::IntegerPosition:
				return "integerposition";

			case PropertyType::IntegerPosition3D:
				return "integerposition3d";

			case PropertyType::IntegerRect:
				return "integerrect";

			case PropertyType::IntegerSize:
				return "integersize";

			case PropertyType::IntegerSize3D:
				return "integersize3d";

			case PropertyType::Layer:
				return "layer";

			case PropertyType::String:
				return "string";

			case PropertyType::Texture:
				return "texture";
		}

		assert(!"Unhandled property type");
		return nullptr;
	}

	PropertyValue TranslatePropertyFromLua(SharedMatch* match, const sol::object& value, PropertyType expectedType, bool isArray)
	{
		auto TranslateEntityToUniqueId = [&](const sol::object& value) -> EntityId
		{
			if (value.is<EntityId>())
			{
				EntityId intValue = value.as<EntityId>();
				if (intValue != InvalidEntityId)
					throw std::runtime_error("Entity property must be entities or InvalidEntityId constant");

				return intValue;
			}
			else
			{
				if (!match)
					throw std::runtime_error("This context only accepts InvalidEntityId constant");

				const Ndk::EntityHandle& entity = AbstractElementLibrary::RetrieveScriptEntity(value);
				return match->RetrieveUniqueIdByEntity(entity);
			}
		};

		if (isArray)
		{
			sol::table content = value.as<sol::table>();
			std::size_t elementCount = content.size();
			if (elementCount == 0)
				throw std::runtime_error("Property array must contain at least one element");

			auto HandleDataArray = [&](auto dummyType) -> PropertyValue
			{
				using T = std::decay_t<decltype(dummyType)>;

				PropertyArrayValue<T::Property> container(elementCount);
				for (std::size_t i = 0; i < elementCount; ++i)
					container[i] = content[i + 1];

				return container;
			};

			switch (expectedType)
			{
				case PropertyType::Bool:
					return HandleDataArray(PropertyTag<PropertyType::Bool>{});

				case PropertyType::Entity:
				{
					PropertyArrayValue<PropertyType::Entity> container(elementCount);
					for (std::size_t i = 0; i < elementCount; ++i)
						container[i] = TranslateEntityToUniqueId(content[i + 1]);

					return container;
				}

				case PropertyType::Float:
					return HandleDataArray(PropertyTag<PropertyType::Float>{});

				case PropertyType::FloatPosition:
					return HandleDataArray(PropertyTag<PropertyType::FloatPosition>{});

				case PropertyType::FloatPosition3D:
					return HandleDataArray(PropertyTag<PropertyType::FloatPosition3D>{});

				case PropertyType::FloatRect:
				{
					PropertyArrayValue<PropertyType::FloatRect> container(elementCount);
					for (std::size_t i = 0; i < elementCount; ++i)
						container[i] = TranslateRectToVec<float>(content[i + 1]);

					return container;
				}

				case PropertyType::FloatSize:
					return HandleDataArray(PropertyTag<PropertyType::FloatSize>{});

				case PropertyType::FloatSize3D:
					return HandleDataArray(PropertyTag<PropertyType::FloatSize3D>{});

				case PropertyType::Integer:
					return HandleDataArray(PropertyTag<PropertyType::Integer>{});

				case PropertyType::IntegerPosition:
					return HandleDataArray(PropertyTag<PropertyType::IntegerPosition>{});

				case PropertyType::IntegerSize:
					return HandleDataArray(PropertyTag<PropertyType::IntegerSize>{});

				case PropertyType::IntegerPosition3D:
					return HandleDataArray(PropertyTag<PropertyType::IntegerPosition3D>{});

				case PropertyType::IntegerSize3D:
					return HandleDataArray(PropertyTag<PropertyType::IntegerSize3D>{});

				case PropertyType::IntegerRect:
				{
					PropertyArrayValue<PropertyType::IntegerRect> container(elementCount);
					for (std::size_t i = 0; i < elementCount; ++i)
						container[i] = TranslateRectToVec<EntityId>(content[i + 1]);

					return container;
				}

				case PropertyType::Layer:
					return HandleDataArray(PropertyTag<PropertyType::Layer>{});

				case PropertyType::String:
					return HandleDataArray(PropertyTag<PropertyType::String>{});

				case PropertyType::Texture:
					return HandleDataArray(PropertyTag<PropertyType::Texture>{});
			}
		}
		else
		{
			auto HandleData = [&](auto dummyType) -> PropertyValue
			{
				using T = std::decay_t<decltype(dummyType)>;
				using SingleValue = PropertySingleValue<T::Property>;

				return PropertySingleValue<T::Property>{ value.as<SingleValue::UnderlyingType>() };
			};

			switch (expectedType)
			{
				case PropertyType::Bool:
					return HandleData(PropertyTag<PropertyType::Bool>{});

				case PropertyType::Entity:
					return PropertySingleValue<PropertyType::Entity>{ TranslateEntityToUniqueId(value) };

				case PropertyType::Float:
					return HandleData(PropertyTag<PropertyType::Float>{});

				case PropertyType::FloatPosition:
					return HandleData(PropertyTag<PropertyType::FloatPosition>{});

				case PropertyType::FloatPosition3D:
					return HandleData(PropertyTag<PropertyType::FloatPosition3D>{});

				case PropertyType::FloatRect:
					return PropertySingleValue<PropertyType::FloatRect>{ TranslateRectToVec<float>(value.as<Nz::Rectf>()) };

				case PropertyType::FloatSize:
					return HandleData(PropertyTag<PropertyType::FloatSize>{});

				case PropertyType::FloatSize3D:
					return HandleData(PropertyTag<PropertyType::FloatSize3D>{});

				case PropertyType::Integer:
					return HandleData(PropertyTag<PropertyType::Integer>{});

				case PropertyType::IntegerPosition:
					return HandleData(PropertyTag<PropertyType::IntegerPosition>{});

				case PropertyType::IntegerPosition3D:
					return HandleData(PropertyTag<PropertyType::IntegerPosition3D>{});

				case PropertyType::IntegerRect:
					return PropertySingleValue<PropertyType::IntegerRect>{ TranslateRectToVec<Nz::Int64>(value.as<Nz::Recti64>()) };

				case PropertyType::IntegerSize:
					return HandleData(PropertyTag<PropertyType::IntegerSize>{});

				case PropertyType::IntegerSize3D:
					return HandleData(PropertyTag<PropertyType::IntegerSize3D>{});

				case PropertyType::Layer:
					return PropertySingleValue<PropertyType::Layer>{ value.as<LayerIndex>() };

				case PropertyType::String:
					return HandleData(PropertyTag<PropertyType::String>{});

				case PropertyType::Texture:
					return HandleData(PropertyTag<PropertyType::Texture>{});
			}
		}

		throw std::runtime_error("Unhandled type");
	}

	sol::object TranslatePropertyToLua(SharedMatch* match, sol::state_view& lua, const PropertyValue& property, PropertyType propertyType)
	{
		return std::visit([&](auto&& value) -> sol::object
		{
			using T = std::decay_t<decltype(value)>;
			using PropertyTypeExtractor = PropertyTypeExtractor<T>;
			using UnderlyingType = PropertyTypeExtractor::UnderlyingType;
			constexpr bool IsArray = PropertyTypeExtractor::IsArray;

			if constexpr (std::is_same_v<UnderlyingType, bool> ||
			              std::is_same_v<UnderlyingType, float> ||
			              std::is_same_v<UnderlyingType, EntityId> ||
			              std::is_same_v<UnderlyingType, LayerIndex> ||
			              std::is_same_v<UnderlyingType, Nz::Int64> ||
			              std::is_same_v<UnderlyingType, Nz::Vector2f> ||
			              std::is_same_v<UnderlyingType, Nz::Vector2i64> ||
			              std::is_same_v<UnderlyingType, Nz::Vector3f> ||
			              std::is_same_v<UnderlyingType, Nz::Vector3i64> ||
			              std::is_same_v<UnderlyingType, Nz::Vector4f> ||
			              std::is_same_v<UnderlyingType, Nz::Vector4i64> ||
			              std::is_same_v<UnderlyingType, std::string>)
			{
				switch (propertyType)
				{
					case PropertyType::Entity:
					{
						if constexpr (std::is_same_v<UnderlyingType, EntityId>)
						{
							auto PushEntity = [&](EntityId uniqueId) -> sol::object
							{
								if (uniqueId == InvalidEntityId)
									return sol::make_object(lua, InvalidEntityId);

								const Ndk::EntityHandle& entity = match->RetrieveEntityByUniqueId(uniqueId);
								if (!entity || !entity->HasComponent<ScriptComponent>())
									return sol::make_object(lua, InvalidEntityId);

								auto& entityScript = entity->GetComponent<ScriptComponent>();
								return entityScript.GetTable();
							};

							return PushValue<IsArray>(lua, value, PushEntity);
						}
						else
							static_assert(AlwaysFalse<T>(), "Property type doesn't match internal type");

						break;
					}

					case PropertyType::FloatRect:
					{
						if constexpr (std::is_same_v<UnderlyingType, Nz::Vector4f>)
						{
							auto PushRect = [&](const Nz::Vector4f& vec) -> sol::object
							{
								return sol::make_object(lua, TranslateVecToRect<float>(vec));
							};

							return PushValue<IsArray>(lua, value, PushRect);
						}
						else
							static_assert(AlwaysFalse<T>(), "Property type doesn't match internal type");
					}

					case PropertyType::IntegerRect:
					{
						if constexpr (std::is_same_v<UnderlyingType, Nz::Vector4i64>)
						{
							auto PushRect = [&](const Nz::Vector4i64& vec) -> sol::object
							{
								return sol::make_object(lua, TranslateVecToRect<Nz::Int64>(vec));
							};

							return PushValue<IsArray>(lua, value, PushRect);
						}
						else
							static_assert(AlwaysFalse<T>(), "Property type doesn't match internal type");
					}

					default:
						return PushValuePassthrough<IsArray>(lua, value);
				}
			}
			else
				static_assert(AlwaysFalse<T>(), "non-exhaustive visitor");

		}, property);
	}
}
