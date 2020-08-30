// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_PROPERTYVALUES_HPP
#define BURGWAR_CORELIB_PROPERTYVALUES_HPP

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
	constexpr Nz::Int64 NoEntity = 0;

	enum class PropertyType
	{
		Bool              = 0,
		Entity            = 1,
		Float             = 2,
		FloatPosition     = 3,
		FloatPosition3D   = 4,
		FloatRect         = 5,
		FloatSize         = 6,
		FloatSize3D       = 7,
		Integer           = 8,
		IntegerPosition   = 9,
		IntegerPosition3D = 10,
		IntegerRect       = 11,
		IntegerSize       = 12,
		IntegerSize3D     = 13,
		Layer             = 14,
		String            = 15,
		Texture           = 16
	};

	enum class PropertyInternalType
	{
		Bool     = 0,
		Float    = 1,
		Float2   = 2,
		Float3   = 3,
		Float4   = 4,
		Integer  = 5,
		Integer2 = 6,
		Integer3 = 7,
		Integer4 = 8,
		String   = 9
	};

	template<typename T> 
	class PropertyArray
	{
		public:
			using StoredType = T;

			explicit PropertyArray(std::size_t elementCount);
			PropertyArray(const PropertyArray&);
			PropertyArray(PropertyArray&&) noexcept = default;
			~PropertyArray() = default;

			T& GetElement(std::size_t i);
			const T& GetElement(std::size_t i) const;
			std::size_t GetSize() const;

			T& operator[](std::size_t i);
			const T& operator[](std::size_t i) const;

			// To allow range-for loops
			T* begin() const;
			T* end() const;
			std::size_t size() const;

			PropertyArray& operator=(const PropertyArray&);
			PropertyArray& operator=(PropertyArray&&) noexcept = default;

		private:
			std::size_t m_size;
			std::unique_ptr<T[]> m_arrayData;
	};

	using PropertyValue = std::variant<bool, PropertyArray<bool>,
	                                    float, PropertyArray<float>,
	                                    Nz::Int64, PropertyArray<Nz::Int64>,
	                                    Nz::Vector2f, PropertyArray<Nz::Vector2f>,
	                                    Nz::Vector2i64, PropertyArray<Nz::Vector2i64>,
	                                    Nz::Vector3f, PropertyArray<Nz::Vector3f>,
	                                    Nz::Vector3i64, PropertyArray<Nz::Vector3i64>,
	                                    Nz::Vector4f, PropertyArray<Nz::Vector4f>,
	                                    Nz::Vector4i64, PropertyArray<Nz::Vector4i64>,
	                                    std::string, PropertyArray<std::string>
	>;
	
	using PropertyValueMap = tsl::hopscotch_map<std::string /*propertyName*/, PropertyValue /*property*/>;

	std::pair<PropertyInternalType, bool> ExtractPropertyType(const PropertyValue& property);

	PropertyType ParsePropertyType(const std::string_view& str);
	PropertyInternalType ParsePropertyInternalType(const std::string_view& str);

	const char* ToString(PropertyType propertyType);
	const char* ToString(PropertyInternalType propertyType);

	PropertyValue TranslatePropertyFromLua(SharedMatch* match, const sol::object& value, PropertyType expectedType, bool isArray);
	sol::object TranslatePropertyToLua(SharedMatch* match, sol::state_view& lua, const PropertyValue& property, PropertyType propertyType);

	template<typename T> Nz::Vector4<T> TranslateRectToVec(const Nz::Rect<T>& value);
	template<typename T> Nz::Rect<T> TranslateVecToRect(const Nz::Vector4<T>& value);
}

#include <CoreLib/PropertyValues.inl>

#endif
