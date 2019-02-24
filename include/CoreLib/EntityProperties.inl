// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/EntityProperties.hpp>
#include <cassert>

namespace bw
{
	template<typename T>
	EntityPropertyArray<T>::EntityPropertyArray(std::size_t elementCount) :
	m_size(elementCount)
	{
		m_arrayData = std::make_unique<T[]>(m_size);
	}

	template<typename T>
	EntityPropertyArray<T>::EntityPropertyArray(const EntityPropertyArray& container)
	{
		*this = container;
	}

	template<typename T>
	T& EntityPropertyArray<T>::GetElement(std::size_t i)
	{
		assert(i < m_size);
		return m_arrayData[i];
	}

	template<typename T>
	const T& EntityPropertyArray<T>::GetElement(std::size_t i) const
	{
		assert(i < m_size);
		return m_arrayData[i];
	}

	template<typename T>
	std::size_t EntityPropertyArray<T>::GetSize() const
	{
		return m_size;
	}

	template<typename T>
	T& EntityPropertyArray<T>::operator[](std::size_t i)
	{
		return GetElement(i);
	}

	template<typename T>
	const T& EntityPropertyArray<T>::operator[](std::size_t i) const
	{
		return GetElement(i);
	}

	template<typename T>
	inline T* EntityPropertyArray<T>::begin() const
	{
		return &m_arrayData[0];
	}

	template<typename T>
	inline T* EntityPropertyArray<T>::end() const
	{
		return &m_arrayData[m_size];
	}

	template<typename T>
	std::size_t EntityPropertyArray<T>::size() const
	{
		return GetSize();
	}

	template<typename T>
	EntityPropertyArray<T>& EntityPropertyArray<T>::operator=(const EntityPropertyArray& container)
	{
		if (this == &container)
			return *this;

		m_size = container.m_size;
		m_arrayData = std::make_unique<T[]>(m_size);
		std::copy(container.m_arrayData.get(), container.m_arrayData.get() + m_size, m_arrayData.get());

		return *this;
	}
}
