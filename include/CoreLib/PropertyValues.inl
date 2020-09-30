// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/PropertyValues.hpp>
#include <cassert>

namespace bw
{
#define BURGWAR_PROPERTYTYPE(V, T, UT) \
	template<> \
	struct PropertyUnderlyingType< PropertyType:: T > \
	{ \
		using UnderlyingType = UT; \
	};

#include <CoreLib/PropertyTypeList.hpp>

	template<PropertyType P>
	struct PropertyTypeExtractor<PropertySingleValue<P>>
	{
		static constexpr PropertyType Property = P;
		static constexpr bool IsArray = false;
		using UnderlyingType = PropertyUnderlyingType_t<P>;
	};

	template<PropertyType P>
	struct PropertyTypeExtractor<PropertyArrayValue<P>>
	{
		static constexpr PropertyType Property = P;
		static constexpr bool IsArray = true;
		using UnderlyingType = PropertyUnderlyingType_t<P>;
	};

	template<PropertyType P>
	PropertyArrayValue<P>::PropertyArrayValue(std::size_t elementCount) :
	m_size(elementCount)
	{
		m_arrayData = std::make_unique<UnderlyingType[]>(m_size);
	}

	template<PropertyType P>
	PropertyArrayValue<P>::PropertyArrayValue(const PropertyArrayValue& container)
	{
		*this = container;
	}

	template<PropertyType P>
	auto PropertyArrayValue<P>::GetElement(std::size_t i) -> UnderlyingType&
	{
		assert(i < m_size);
		return m_arrayData[i];
	}

	template<PropertyType P>
	auto PropertyArrayValue<P>::GetElement(std::size_t i) const -> const UnderlyingType&
	{
		assert(i < m_size);
		return m_arrayData[i];
	}

	template<PropertyType P>
	std::size_t PropertyArrayValue<P>::GetSize() const
	{
		return m_size;
	}

	template<PropertyType P>
	auto PropertyArrayValue<P>::operator[](std::size_t i) -> UnderlyingType&
	{
		return GetElement(i);
	}

	template<PropertyType P>
	auto PropertyArrayValue<P>::operator[](std::size_t i) const -> const UnderlyingType&
	{
		return GetElement(i);
	}

	template<PropertyType P>
	auto PropertyArrayValue<P>::begin() -> UnderlyingType*
	{
		return &m_arrayData[0];
	}

	template<PropertyType P>
	auto PropertyArrayValue<P>::begin() const -> const UnderlyingType*
	{
		return &m_arrayData[0];
	}

	template<PropertyType P>
	auto PropertyArrayValue<P>::end() -> UnderlyingType*
	{
		return &m_arrayData[m_size];
	}

	template<PropertyType P>
	auto PropertyArrayValue<P>::end() const -> const UnderlyingType*
	{
		return &m_arrayData[m_size];
	}

	template<PropertyType P>
	std::size_t PropertyArrayValue<P>::size() const
	{
		return GetSize();
	}

	template<PropertyType P>
	PropertyArrayValue<P>& PropertyArrayValue<P>::operator=(const PropertyArrayValue& container)
	{
		if (this == &container)
			return *this;

		m_size = container.m_size;
		m_arrayData = std::make_unique<UnderlyingType[]>(m_size);
		std::copy(container.m_arrayData.get(), container.m_arrayData.get() + m_size, m_arrayData.get());

		return *this;
	}


	template<PropertyType P>
	PropertySingleValue<P>::PropertySingleValue(const UnderlyingType& v) :
	value(v)
	{
	}

	template<PropertyType P>
	PropertySingleValue<P>::PropertySingleValue(UnderlyingType&& v) :
	value(std::move(v))
	{
	}

	template<PropertyType P>
	auto PropertySingleValue<P>::operator*() & -> UnderlyingType&
	{
		return value;
	}

	template<PropertyType P>
	auto PropertySingleValue<P>::operator*() && -> UnderlyingType&&
	{
		return std::move(value);
	}

	template<PropertyType P>
	auto PropertySingleValue<P>::operator*() const & -> const UnderlyingType&
	{
		return value;
	}


	template<typename T>
	Nz::Vector4<T> TranslateRectToVec(const Nz::Rect<T>& value)
	{
		return { value.x, value.y, value.width, value.height };
	};

	template<typename T>
	Nz::Rect<T> TranslateVecToRect(const Nz::Vector4<T>& value)
	{
		return { value.x, value.y, value.z, value.w };
	};
}
