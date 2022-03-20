// Copyright (C) 2016 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in Prerequisites.hpp

#include <CoreLib/PropertyValues.hpp>
#include <CoreLib/SharedMatch.hpp>
#include <CoreLib/Components/ScriptComponent.hpp>
#include <CoreLib/Scripting/AbstractElementLibrary.hpp>
#include <CoreLib/Scripting/AbstractScriptingLibrary.hpp>
#include <CoreLib/Scripting/ScriptingUtils.hpp>
#include <CoreLib/Utils.hpp>

namespace bw
{
	namespace
	{
		template<PropertyType P>
		struct PropertyGetter
		{
			using UnderlyingType = PropertyUnderlyingType_t<P>;

			UnderlyingType operator()(SharedMatch* /*match*/, const sol::object& value)
			{
				return value.as<UnderlyingType>();
			}
		};

		template<>
		struct PropertyGetter<PropertyType::Entity>
		{
			using UnderlyingType = PropertyUnderlyingType_t<PropertyType::Entity>;

			UnderlyingType operator()(SharedMatch* match, const sol::object& value)
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

					const Ndk::EntityHandle& entity = RetrieveScriptEntity(value);
					return match->RetrieveUniqueIdByEntity(entity);
				}
			}
		};

		template<>
		struct PropertyGetter<PropertyType::FloatRect>
		{
			using UnderlyingType = PropertyUnderlyingType_t<PropertyType::FloatRect>;

			UnderlyingType operator()(SharedMatch* /*match*/, const sol::object& value)
			{
				return TranslateRectToVec(value.as<Nz::Rectf>());
			}
		};

		template<>
		struct PropertyGetter<PropertyType::IntegerRect>
		{
			using UnderlyingType = PropertyUnderlyingType_t<PropertyType::IntegerRect>;

			UnderlyingType operator()(SharedMatch* /*match*/, const sol::object& value)
			{
				return TranslateRectToVec(value.as<Nz::Recti64>());
			}
		};


		template<PropertyType P>
		struct PropertyPusher
		{
			template<typename T>
			sol::object operator()(SharedMatch* /*match*/, sol::state_view& lua, T&& value)
			{
				return sol::make_object(lua, std::forward<T>(value));
			}
		};

		template<>
		struct PropertyPusher<PropertyType::Entity>
		{
			sol::object operator()(SharedMatch* match, sol::state_view& lua, EntityId uniqueId)
			{
				if (uniqueId == InvalidEntityId)
					return sol::make_object(lua, InvalidEntityId);

				const Ndk::EntityHandle& entity = match->RetrieveEntityByUniqueId(uniqueId);
				if (!entity || !entity->HasComponent<ScriptComponent>())
					return sol::make_object(lua, InvalidEntityId);

				auto& entityScript = entity->GetComponent<ScriptComponent>();
				return sol::table(lua, entityScript.GetTable());
			}
		};

		template<>
		struct PropertyPusher<PropertyType::FloatRect>
		{
			sol::object operator()(SharedMatch* /*match*/, sol::state_view& lua, const Nz::Vector4f& rect)
			{
				return sol::make_object(lua, TranslateVecToRect<float>(rect));
			}
		};

		template<>
		struct PropertyPusher<PropertyType::IntegerRect>
		{
			sol::object operator()(SharedMatch* /*match*/, sol::state_view& lua, const Nz::Vector4i64& rect)
			{
				return sol::make_object(lua, TranslateVecToRect<Nz::Int64>(rect));
			}
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
#define BURGWAR_PROPERTYTYPE(V, T, IT) if (StringEqual(str, #T)) return PropertyType:: T;

#include <CoreLib/PropertyTypeList.hpp>

		throw std::runtime_error("Invalid property type " + std::string(str));
	}

	const char* ToString(PropertyType propertyType)
	{
		switch (propertyType)
		{
#define BURGWAR_PROPERTYTYPE(V, T, IT) case PropertyType:: T: return #T ;

#include <CoreLib/PropertyTypeList.hpp>
		}

		assert(!"Unhandled property type");
		return nullptr;
	}

	PropertyValue TranslatePropertyFromLua(SharedMatch* match, const sol::object& value, PropertyType expectedType, bool isArray)
	{
		if (isArray)
		{
			sol::table content = value.as<sol::table>();
			std::size_t elementCount = content.size();

			auto HandleDataArray = [&](auto dummyType) -> PropertyValue
			{
				using T = std::decay_t<decltype(dummyType)>;

				PropertyArrayValue<T::Property> container(elementCount);
				PropertyGetter<T::Property> getter;
				for (std::size_t i = 0; i < elementCount; ++i)
					container[i] = getter(match, content[i + 1]);

				return container;
			};

			switch (expectedType)
			{
#define BURGWAR_PROPERTYTYPE(V, T, IT) case PropertyType:: T: return HandleDataArray(PropertyTag<PropertyType:: T>{});

#include <CoreLib/PropertyTypeList.hpp>
			}
		}
		else
		{
			auto HandleData = [&](auto dummyType) -> PropertyValue
			{
				using T = std::decay_t<decltype(dummyType)>;
				
				PropertyGetter<T::Property> getter;
				return PropertySingleValue<T::Property>{ getter(match, value) };
			};

			switch (expectedType)
			{
#define BURGWAR_PROPERTYTYPE(V, T, IT) case PropertyType:: T: return HandleData(PropertyTag<PropertyType:: T>{});

#include <CoreLib/PropertyTypeList.hpp>
			}
		}

		throw std::runtime_error("Unhandled type");
	}

	sol::object TranslatePropertyToLua(SharedMatch* match, sol::state_view& lua, const PropertyValue& property)
	{
		return std::visit([&](auto&& value) -> sol::object
		{
			using T = std::decay_t<decltype(value)>;
			using PropertyTypeExtractor = PropertyTypeExtractor<T>;
			constexpr bool IsArray = PropertyTypeExtractor::IsArray;

			PropertyPusher<PropertyTypeExtractor::Property> pusher;

			if constexpr (IsArray)
			{
				std::size_t elementCount = value.GetSize();
				sol::table content = lua.create_table(int(elementCount));

				for (std::size_t i = 0; i < elementCount; ++i)
					content[i + 1] = pusher(match, lua, value[i]);

				return content;
			}
			else
				return pusher(match, lua, *value);

		}, property);
	}
}
