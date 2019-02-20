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
	class EntityPropertyContainer
	{
		public:
			using StoredType = T;

			explicit EntityPropertyContainer(bool isArray, std::size_t elementCount);
			explicit EntityPropertyContainer(const T& value);
			explicit EntityPropertyContainer(T&& value);
			EntityPropertyContainer(const EntityPropertyContainer&);
			EntityPropertyContainer(EntityPropertyContainer&&) noexcept = default;
			~EntityPropertyContainer() = default;

			bool IsArray() const;

			T& GetElement(std::size_t i);
			const T& GetElement(std::size_t i) const;
			std::size_t GetSize() const;

			EntityPropertyContainer& operator=(const EntityPropertyContainer&);
			EntityPropertyContainer& operator=(EntityPropertyContainer&&) noexcept = default;

		private:
			std::optional<T> m_singleData;
			std::size_t m_elementCount;
			std::unique_ptr<T[]> m_arrayData;
	};

	using EntityProperty = std::variant<std::monostate, EntityPropertyContainer<bool>, EntityPropertyContainer<float>, EntityPropertyContainer<Nz::Int64>, EntityPropertyContainer<std::string>>;
	using EntityProperties = tsl::hopscotch_map<std::string /*propertyName*/, EntityProperty /*property*/>;
}

#include <CoreLib/EntityProperties.inl>

#endif
