// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/EntityProperties.hpp>
#include <cassert>

namespace bw
{
	template<typename T>
	EntityPropertyContainer<T>::EntityPropertyContainer(bool isArray, std::size_t elementCount) :
	m_elementCount(elementCount)
	{
		if (isArray)
			m_arrayData = std::make_unique<T[]>(m_elementCount);
		else
		{
			assert(m_elementCount == 1);
			m_singleData.emplace();
		}
	}

	template<typename T>
	EntityPropertyContainer<T>::EntityPropertyContainer(const T& value) :
	m_elementCount(1)
	{
		m_singleData.emplace(value);
	}

	template<typename T>
	EntityPropertyContainer<T>::EntityPropertyContainer(T&& value) :
	m_elementCount(1)
	{
		m_singleData.emplace(std::move(value));
	}

	template<typename T>
	EntityPropertyContainer<T>::EntityPropertyContainer(const EntityPropertyContainer& container)
	{
		*this = container;
	}

	template<typename T>
	T& EntityPropertyContainer<T>::GetElement(std::size_t i)
	{
		if (IsArray())
		{
			assert(i < m_elementCount);
			return m_arrayData[i];
		}
		else
		{
			assert(i == 0);
			return m_singleData.value();
		}
	}

	template<typename T>
	const T& EntityPropertyContainer<T>::GetElement(std::size_t i) const
	{
		if (IsArray())
		{
			assert(i < m_elementCount);
			return m_arrayData[i];
		}
		else
		{
			assert(i == 0);
			return m_singleData.value();
		}
	}

	template<typename T>
	std::size_t EntityPropertyContainer<T>::GetSize() const
	{
		return m_elementCount;
	}

	template<typename T>
	EntityPropertyContainer<T>& EntityPropertyContainer<T>::operator=(const EntityPropertyContainer& container)
	{
		if (this == &container)
			return *this;

		m_elementCount = container.m_elementCount;
		if (container.m_singleData.has_value())
			m_singleData = container.m_singleData;
		else
		{
			m_arrayData = std::make_unique<T[]>(m_elementCount);
			std::copy(container.m_arrayData.get(), container.m_arrayData.get() + m_elementCount, m_arrayData.get());
		}

		return *this;
	}

	template<typename T>
	bool EntityPropertyContainer<T>::IsArray() const
	{
		return !m_singleData.has_value();
	}
}
