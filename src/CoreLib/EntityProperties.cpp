// Copyright (C) 2016 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in Prerequisites.hpp

#include <CoreLib/EntityProperties.hpp>
#include <CoreLib/SharedMatch.hpp>
#include <CoreLib/Components/ScriptComponent.hpp>
#include <CoreLib/Scripting/AbstractElementLibrary.hpp>
#include <CoreLib/Scripting/AbstractScriptingLibrary.hpp>
#include <CoreLib/Utils.hpp>

namespace bw
{
	namespace
	{
		template<typename T, typename ConvertFunc>
		sol::object PushValue(sol::state_view& lua, T&& value, ConvertFunc&& convertFunc)
		{
			using Type = std::decay_t<decltype(value)>;
			constexpr bool IsArray = IsSameTpl_v<EntityPropertyArray, Type>;

			if constexpr (IsArray)
			{
				std::size_t elementCount = value.GetSize();
				sol::table content = lua.create_table(int(elementCount));

				for (std::size_t i = 0; i < elementCount; ++i)
					content[i + 1] = convertFunc(value[i]);

				return content;
			}
			else
				return convertFunc(value);
		};

		template<typename T>
		sol::object PushValuePassthrough(sol::state_view& lua, T&& value)
		{
			// I had troubles using a passthrough (std::forward) function with PushValue
			/*constexpr bool IsArray = IsSameTpl_v<EntityPropertyArray, T>;
			using InternalType = std::conditional_t<IsArray, typename IsSameTpl<EntityPropertyArray, T>::ContainedType, T>;

			return PushValue(lua, value, [&lua](InternalType&& val) -> sol::object
			{
				return sol::make_object(lua, std::forward<InternalType>(val));
			});*/

			using Type = std::decay_t<decltype(value)>;
			constexpr bool IsArray = IsSameTpl_v<EntityPropertyArray, Type>;

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
		};
	}

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

			static_assert(std::is_same_v<std::variant_alternative_t<10, EntityProperty>, Nz::Vector3f>);
			static_assert(std::is_same_v<std::variant_alternative_t<11, EntityProperty>, EntityPropertyArray<Nz::Vector3f>>);

			case 10:
				return std::make_pair(PropertyInternalType::Float3, false);
			case 11:
				return std::make_pair(PropertyInternalType::Float3, true);

			static_assert(std::is_same_v<std::variant_alternative_t<12, EntityProperty>, Nz::Vector3i64>);
			static_assert(std::is_same_v<std::variant_alternative_t<13, EntityProperty>, EntityPropertyArray<Nz::Vector3i64>>);

			case 12:
				return std::make_pair(PropertyInternalType::Integer3, false);
			case 13:
				return std::make_pair(PropertyInternalType::Integer3, true);

			static_assert(std::is_same_v<std::variant_alternative_t<14, EntityProperty>, Nz::Vector4f>);
			static_assert(std::is_same_v<std::variant_alternative_t<15, EntityProperty>, EntityPropertyArray<Nz::Vector4f>>);

			case 14:
				return std::make_pair(PropertyInternalType::Float4, false);
			case 15:
				return std::make_pair(PropertyInternalType::Float4, true);

			static_assert(std::is_same_v<std::variant_alternative_t<16, EntityProperty>, Nz::Vector4i64>);
			static_assert(std::is_same_v<std::variant_alternative_t<17, EntityProperty>, EntityPropertyArray<Nz::Vector4i64>>);

			case 16:
				return std::make_pair(PropertyInternalType::Integer4, false);
			case 17:
				return std::make_pair(PropertyInternalType::Integer4, true);

			static_assert(std::is_same_v<std::variant_alternative_t<18, EntityProperty>, std::string>);
			static_assert(std::is_same_v<std::variant_alternative_t<19, EntityProperty>, EntityPropertyArray<std::string>>);

			case 18:
				return std::make_pair(PropertyInternalType::String, false);
			case 19:
				return std::make_pair(PropertyInternalType::String, true);
		}

		throw std::runtime_error("Unhandled internal type");
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
		else if (str == "floatrect")
			return PropertyType::FloatRect;
		else if (str == "floatsize")
			return PropertyType::FloatSize;
		else if (str == "int" || str == "integer")
			return PropertyType::Integer;
		else if (str == "intpos" || str == "integerposition")
			return PropertyType::IntegerPosition;
		else if (str == "intrect")
			return PropertyType::IntegerRect;
		else if (str == "intsize" || str == "integersize")
			return PropertyType::IntegerSize;
		else if (str == "layer")
			return PropertyType::Layer;
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
		else if (str == "float3")
			return PropertyInternalType::Float3;
		else if (str == "float4")
			return PropertyInternalType::Float4;
		else if (str == "integer")
			return PropertyInternalType::Integer;
		else if (str == "integer2")
			return PropertyInternalType::Integer2;
		else if (str == "integer3")
			return PropertyInternalType::Integer3;
		else if (str == "integer4")
			return PropertyInternalType::Integer4;
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

			case PropertyType::Entity:
				return "entity";

			case PropertyType::Float:
				return "float";

			case PropertyType::FloatPosition:
				return "floatposition";

			case PropertyType::FloatRect:
				return "floatrect";

			case PropertyType::FloatSize:
				return "floatsize";

			case PropertyType::Integer:
				return "integer";

			case PropertyType::IntegerPosition:
				return "integerposition";

			case PropertyType::IntegerRect:
				return "integerrect";

			case PropertyType::IntegerSize:
				return "integersize";

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

			case PropertyInternalType::Float3:
				return "float3";

			case PropertyInternalType::Float4:
				return "float4";

			case PropertyInternalType::Integer:
				return "integer";

			case PropertyInternalType::Integer2:
				return "integer2";

			case PropertyInternalType::Integer3:
				return "integer3";

			case PropertyInternalType::Integer4:
				return "integer4";

			case PropertyInternalType::String:
				return "string";
		}

		assert(!"Unhandled property internal type");
		return nullptr;
	}

	EntityProperty TranslateEntityPropertyFromLua(SharedMatch* match, const sol::object& value, PropertyType expectedType, bool isArray)
	{
		auto TranslateEntityToUniqueId = [&](const sol::object& value) -> Nz::Int64
		{
			if (value.is<Nz::Int64>())
			{
				Nz::Int64 intValue = value.as<Nz::Int64>();
				if (intValue != NoEntity)
					throw std::runtime_error("Entity property must be entities or NoEntity constant");

				return intValue;
			}
			else
			{
				if (!match)
					throw std::runtime_error("This context only accepts NoEntity constant");

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

				case PropertyType::Entity:
				{
					EntityPropertyArray<Nz::Int64> container(elementCount);
					for (std::size_t i = 0; i < elementCount; ++i)
						container[i] = TranslateEntityToUniqueId(content[i + 1]);

					return container;
				}

				case PropertyType::Integer:
				case PropertyType::Layer:
					return HandleDataArray(Nz::Int64());

				case PropertyType::Float:
					return HandleDataArray(float());

				case PropertyType::FloatPosition:
				case PropertyType::FloatSize:
					return HandleDataArray(Nz::Vector2f());

				case PropertyType::FloatRect:
				{
					EntityPropertyArray<Nz::Vector4f> container(elementCount);
					for (std::size_t i = 0; i < elementCount; ++i)
						container[i] = TranslateRectToVec<float>(content[i + 1]);

					return container;
				}

				case PropertyType::IntegerPosition:
				case PropertyType::IntegerSize:
					return HandleDataArray(Nz::Vector2i64());

				case PropertyType::IntegerRect:
				{
					EntityPropertyArray<Nz::Vector4i64> container(elementCount);
					for (std::size_t i = 0; i < elementCount; ++i)
						container[i] = TranslateRectToVec<Nz::Int64>(content[i + 1]);

					return container;
				}

				case PropertyType::String:
				case PropertyType::Texture:
					return HandleDataArray(std::string());
			}
		}
		else
		{
			switch (expectedType)
			{
				case PropertyType::Bool:
					return value.as<bool>();

				case PropertyType::Entity:
					return TranslateEntityToUniqueId(value);

				case PropertyType::Float:
					return value.as<float>();

				case PropertyType::FloatPosition:
				case PropertyType::FloatSize:
					return value.as<Nz::Vector2f>();

				case PropertyType::FloatRect:
					return TranslateRectToVec<float>(value.as<Nz::Rectf>());

				case PropertyType::Integer:
				case PropertyType::Layer:
					return value.as<Nz::Int64>();

				case PropertyType::IntegerPosition:
				case PropertyType::IntegerSize:
					return value.as<Nz::Vector2i64>();

				case PropertyType::IntegerRect:
					return TranslateRectToVec<Nz::Int64>(value.as<Nz::Recti64>());

				case PropertyType::String:
				case PropertyType::Texture:
					return value.as<std::string>();
			}
		}

		throw std::runtime_error("Unhandled type");
	}

	sol::object TranslateEntityPropertyToLua(SharedMatch* match, sol::state_view& lua, const EntityProperty& property, PropertyType propertyType)
	{
		return std::visit([&](auto&& value) -> sol::object
		{
			using T = std::decay_t<decltype(value)>;
			constexpr bool IsArray = IsSameTpl_v<EntityPropertyArray, T>;
			using InternalType = std::conditional_t<IsArray, typename IsSameTpl<EntityPropertyArray, T>::ContainedType, T>;

			if constexpr (std::is_same_v<InternalType, bool> ||
			              std::is_same_v<InternalType, float> ||
			              std::is_same_v<InternalType, Nz::Int64> ||
			              std::is_same_v<InternalType, Nz::Vector2f> ||
			              std::is_same_v<InternalType, Nz::Vector2i64> ||
			              std::is_same_v<InternalType, Nz::Vector3f> ||
			              std::is_same_v<InternalType, Nz::Vector3i64> ||
			              std::is_same_v<InternalType, Nz::Vector4f> ||
			              std::is_same_v<InternalType, Nz::Vector4i64> ||
			              std::is_same_v<InternalType, std::string>)
			{
				switch (propertyType)
				{
					case PropertyType::Entity:
					{
						if constexpr (std::is_same_v<InternalType, Nz::Int64>)
						{
							auto PushEntity = [&](Nz::Int64 uniqueId) -> sol::object
							{
								if (uniqueId == NoEntity)
									return sol::make_object(lua, NoEntity);

								const Ndk::EntityHandle& entity = match->RetrieveEntityByUniqueId(uniqueId);
								if (!entity || !entity->HasComponent<ScriptComponent>())
									return sol::make_object(lua, NoEntity);

								auto& entityScript = entity->GetComponent<ScriptComponent>();
								return entityScript.GetTable();
							};

							return PushValue(lua, value, PushEntity);
						}
						else
							throw std::runtime_error("Property type doesn't match internal type");

						break;
					}

					case PropertyType::FloatRect:
					{
						if constexpr (std::is_same_v<InternalType, Nz::Vector4f>)
						{
							auto PushRect = [&](const Nz::Vector4f& vec) -> sol::object
							{
								return sol::make_object(lua, TranslateVecToRect<float>(vec));
							};

							return PushValue(lua, value, PushRect);
						}
						else
							throw std::runtime_error("Property type doesn't match internal type");
					}

					case PropertyType::IntegerRect:
					{
						if constexpr (std::is_same_v<InternalType, Nz::Vector4i64>)
						{
							auto PushRect = [&](const Nz::Vector4i64& vec) -> sol::object
							{
								return sol::make_object(lua, TranslateVecToRect<Nz::Int64>(vec));
							};

							return PushValue(lua, value, PushRect);
						}
						else
							throw std::runtime_error("Property type doesn't match internal type");
					}

					default:
						return PushValuePassthrough(lua, value);
				}
			}
			else
				static_assert(AlwaysFalse<T>(), "non-exhaustive visitor");

		}, property);
	}
}
