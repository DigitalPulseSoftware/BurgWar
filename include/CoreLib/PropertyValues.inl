// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/PropertyValues.hpp>
#include <cassert>

namespace bw
{
	template<typename T>
	PropertyArray<T>::PropertyArray(std::size_t elementCount) :
	m_size(elementCount)
	{
		m_arrayData = std::make_unique<T[]>(m_size);
	}

	template<typename T>
	PropertyArray<T>::PropertyArray(const PropertyArray& container)
	{
		*this = container;
	}

	template<typename T>
	T& PropertyArray<T>::GetElement(std::size_t i)
	{
		assert(i < m_size);
		return m_arrayData[i];
	}

	template<typename T>
	const T& PropertyArray<T>::GetElement(std::size_t i) const
	{
		assert(i < m_size);
		return m_arrayData[i];
	}

	template<typename T>
	std::size_t PropertyArray<T>::GetSize() const
	{
		return m_size;
	}

	template<typename T>
	T& PropertyArray<T>::operator[](std::size_t i)
	{
		return GetElement(i);
	}

	template<typename T>
	const T& PropertyArray<T>::operator[](std::size_t i) const
	{
		return GetElement(i);
	}

	template<typename T>
	inline T* PropertyArray<T>::begin() const
	{
		return &m_arrayData[0];
	}

	template<typename T>
	inline T* PropertyArray<T>::end() const
	{
		return &m_arrayData[m_size];
	}

	template<typename T>
	std::size_t PropertyArray<T>::size() const
	{
		return GetSize();
	}

	template<typename T>
	PropertyArray<T>& PropertyArray<T>::operator=(const PropertyArray& container)
	{
		if (this == &container)
			return *this;

		m_size = container.m_size;
		m_arrayData = std::make_unique<T[]>(m_size);
		std::copy(container.m_arrayData.get(), container.m_arrayData.get() + m_size, m_arrayData.get());

		return *this;
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
