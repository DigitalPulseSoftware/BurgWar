// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_ENTITYPROPERTIES_HPP
#define BURGWAR_CORELIB_ENTITYPROPERTIES_HPP

#include <Nazara/Prerequisites.hpp>
#include <Nazara/Math/Vector2.hpp>
#include <sol3/sol.hpp>
#include <tsl/hopscotch_map.h>
#include <optional>
#include <memory>
#include <variant>

namespace bw
{
	enum class PropertyType
	{
		Bool            = 0,
		Float           = 1,
		FloatPosition   = 2,
		FloatSize       = 3,
		Integer         = 4,
		IntegerPosition = 5,
		IntegerSize     = 6,
		Layer           = 7,
		String          = 8,
		Texture         = 9
	};

	enum class PropertyInternalType
	{
		Bool     = 0,
		Float    = 1,
		Float2   = 2,
		Integer  = 3,
		Integer2 = 4,
		String   = 5
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
	                                    std::string, EntityPropertyArray<std::string>
	>;
	
	using EntityProperties = tsl::hopscotch_map<std::string /*propertyName*/, EntityProperty /*property*/>;

	constexpr std::size_t MaxPropertyCount = 256;

	std::pair<PropertyInternalType, bool> ExtractPropertyType(const EntityProperty& property);

	PropertyType ParsePropertyType(const std::string_view& str);
	PropertyInternalType ParsePropertyInternalType(const std::string_view& str);

	const char* ToString(PropertyType propertyType);
	const char* ToString(PropertyInternalType propertyType);

	EntityProperty TranslateEntityPropertyFromLua(const sol::object& value, PropertyType expectedType, bool isArray);
	sol::object TranslateEntityPropertyToLua(sol::state_view& lua, const EntityProperty& property);
}

#include <CoreLib/EntityProperties.inl>

#endif
