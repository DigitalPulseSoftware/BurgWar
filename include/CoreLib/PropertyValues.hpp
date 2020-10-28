// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_PROPERTYVALUES_HPP
#define BURGWAR_CORELIB_PROPERTYVALUES_HPP

#include <CoreLib/EntityId.hpp>
#include <CoreLib/LayerIndex.hpp>
#include <Nazara/Prerequisites.hpp>
#include <Nazara/Math/Rect.hpp>
#include <Nazara/Math/Vector2.hpp>
#include <Nazara/Math/Vector3.hpp>
#include <Nazara/Math/Vector4.hpp>
#include <Thirdparty/sol3/sol.hpp>
#include <Thirdparty/tsl/hopscotch_map.h>
#include <optional>
#include <memory>
#include <variant>

namespace bw
{
	class SharedMatch;

	constexpr std::size_t MaxPropertyCount = 256;

	enum class PropertyType
	{
#define BURGWAR_PROPERTYTYPE(V, T, IT) T = V,
#define BURGWAR_PROPERTYTYPE_LAST(V, T, IT) T = V

#include <CoreLib/PropertyTypeList.hpp>
	};

	template<PropertyType P>
	struct PropertyUnderlyingType;

	template<PropertyType P>
	using PropertyUnderlyingType_t = typename PropertyUnderlyingType<P>::UnderlyingType;


	template<typename>
	struct PropertyTypeExtractor;

	template<PropertyType P>
	struct PropertyTag
	{
		static constexpr PropertyType Property = P;
	};

	template<PropertyType P>
	class PropertyArrayValue
	{
		public:
			static constexpr PropertyType Property = P;
			using UnderlyingType = PropertyUnderlyingType_t<Property>;

			explicit PropertyArrayValue(std::size_t elementCount);
			PropertyArrayValue(const PropertyArrayValue&);
			PropertyArrayValue(PropertyArrayValue&&) noexcept = default;
			~PropertyArrayValue() = default;

			UnderlyingType& GetElement(std::size_t i);
			const UnderlyingType& GetElement(std::size_t i) const;
			std::size_t GetSize() const;

			UnderlyingType& operator[](std::size_t i);
			const UnderlyingType& operator[](std::size_t i) const;

			// To allow range-for loops
			UnderlyingType* begin();
			const UnderlyingType* begin() const;
			UnderlyingType* end();
			const UnderlyingType* end() const;
			std::size_t size() const;

			PropertyArrayValue& operator=(const PropertyArrayValue&);
			PropertyArrayValue& operator=(PropertyArrayValue&&) noexcept = default;

		private:
			std::size_t m_size;
			std::unique_ptr<UnderlyingType[]> m_arrayData;
	};

	template<PropertyType P>
	struct PropertySingleValue
	{
		static constexpr PropertyType Property = P;
		using UnderlyingType = PropertyUnderlyingType_t<Property>;

		PropertySingleValue() = default;
		PropertySingleValue(const UnderlyingType& v);
		PropertySingleValue(UnderlyingType&& v);
		PropertySingleValue(const PropertySingleValue&) = default;
		PropertySingleValue(PropertySingleValue&&) = default;

		UnderlyingType& operator*() &;
		UnderlyingType&& operator*() &&;
		const UnderlyingType& operator*() const &;

		PropertySingleValue& operator=(const PropertySingleValue&) = default;
		PropertySingleValue& operator=(PropertySingleValue&&) = default;

		UnderlyingType value;
	};

	using PropertyValue = std::variant<
#define BURGWAR_PROPERTYTYPE(V, T, IT) PropertySingleValue<PropertyType:: T>, PropertyArrayValue<PropertyType:: T>,
#define BURGWAR_PROPERTYTYPE_LAST(V, T, IT) PropertySingleValue<PropertyType:: T>, PropertyArrayValue<PropertyType:: T>

#include <CoreLib/PropertyTypeList.hpp>
	>;
	
	using PropertyValueMap = tsl::hopscotch_map<std::string /*propertyName*/, PropertyValue /*property*/>;

	std::pair<PropertyType, bool> ExtractPropertyType(const PropertyValue& value);

	PropertyType ParsePropertyType(const std::string_view& str);

	const char* ToString(PropertyType propertyType);

	PropertyValue TranslatePropertyFromLua(SharedMatch* match, const sol::object& value, PropertyType expectedType, bool isArray);
	sol::object TranslatePropertyToLua(SharedMatch* match, sol::state_view& lua, const PropertyValue& property);

	template<typename T> Nz::Vector4<T> TranslateRectToVec(const Nz::Rect<T>& value);
	template<typename T> Nz::Rect<T> TranslateVecToRect(const Nz::Vector4<T>& value);
}

#include <CoreLib/PropertyValues.inl>

#endif
