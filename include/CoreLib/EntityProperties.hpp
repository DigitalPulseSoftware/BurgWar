// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_ENTITYPROPERTIES_HPP
#define BURGWAR_CORELIB_ENTITYPROPERTIES_HPP

#include <Nazara/Prerequisites.hpp>
#include <hopscotch/hopscotch_map.h>
#include <optional>
#include <memory>
#include <variant>

namespace bw
{
	enum class PropertyType
	{
		Bool,
		Float,
		Integer,
		String,
		Texture
	};

	PropertyType ParsePropertyType(const std::string_view& str);

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
	                                    std::string, EntityPropertyArray<std::string>>;
	
	using EntityProperties = tsl::hopscotch_map<std::string /*propertyName*/, EntityProperty /*property*/>;
}

#include <CoreLib/EntityProperties.inl>

#endif
