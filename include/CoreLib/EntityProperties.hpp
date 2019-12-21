// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_ENTITYPROPERTIES_HPP
#define BURGWAR_CORELIB_ENTITYPROPERTIES_HPP

#include <Nazara/Prerequisites.hpp>
#include <Nazara/Math/Rect.hpp>
#include <Nazara/Math/Vector2.hpp>
#include <Nazara/Math/Vector3.hpp>
#include <Nazara/Math/Vector4.hpp>
#include <sol3/sol.hpp>
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
		Bool            = 0,
		Entity          = 1,
		Float           = 2,
		FloatPosition   = 3,
		FloatRect       = 4,
		FloatSize       = 5,
		Integer         = 6,
		IntegerPosition = 7,
		IntegerRect     = 8,
		IntegerSize     = 9,
		Layer           = 10,
		String          = 11,
		Texture         = 12
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
	class EntityPropertyArray
	{
		public:
			using StoredType = T;

			explicit EntityPropertyArray(std::size_t elementCount);
			EntityPropertyArray(const EntityPropertyArray&);
			EntityPropertyArray(EntityPropertyArray&&) noexcept = default;
			~EntityPropertyArray() = default;

			T& GetElement(std::size_t i);
			const T& GetElement(std::size_t i) const;
			std::size_t GetSize() const;

			T& operator[](std::size_t i);
			const T& operator[](std::size_t i) const;

			// To allow range-for loops
			T* begin() const;
			T* end() const;
			std::size_t size() const;

			EntityPropertyArray& operator=(const EntityPropertyArray&);
			EntityPropertyArray& operator=(EntityPropertyArray&&) noexcept = default;

		private:
			std::size_t m_size;
			std::unique_ptr<T[]> m_arrayData;
	};

	using EntityProperty = std::variant<bool, EntityPropertyArray<bool>,
	                                    float, EntityPropertyArray<float>,
	                                    Nz::Int64, EntityPropertyArray<Nz::Int64>,
	                                    Nz::Vector2f, EntityPropertyArray<Nz::Vector2f>,
	                                    Nz::Vector2i64, EntityPropertyArray<Nz::Vector2i64>,
	                                    Nz::Vector3f, EntityPropertyArray<Nz::Vector3f>,
	                                    Nz::Vector3i64, EntityPropertyArray<Nz::Vector3i64>,
	                                    Nz::Vector4f, EntityPropertyArray<Nz::Vector4f>,
	                                    Nz::Vector4i64, EntityPropertyArray<Nz::Vector4i64>,
	                                    std::string, EntityPropertyArray<std::string>
	>;
	
	using EntityProperties = tsl::hopscotch_map<std::string /*propertyName*/, EntityProperty /*property*/>;

	std::pair<PropertyInternalType, bool> ExtractPropertyType(const EntityProperty& property);

	PropertyType ParsePropertyType(const std::string_view& str);
	PropertyInternalType ParsePropertyInternalType(const std::string_view& str);

	const char* ToString(PropertyType propertyType);
	const char* ToString(PropertyInternalType propertyType);

	EntityProperty TranslateEntityPropertyFromLua(SharedMatch* match, const sol::object& value, PropertyType expectedType, bool isArray);
	sol::object TranslateEntityPropertyToLua(SharedMatch* match, sol::state_view& lua, const EntityProperty& property, PropertyType propertyType);

	template<typename T> Nz::Vector4<T> TranslateRectToVec(const Nz::Rect<T>& value);
	template<typename T> Nz::Rect<T> TranslateVecToRect(const Nz::Vector4<T>& value);
}

#include <CoreLib/EntityProperties.inl>

#endif
