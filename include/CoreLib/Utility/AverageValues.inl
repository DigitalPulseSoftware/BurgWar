// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Utility/AverageValues.hpp>

namespace bw
{
	template<typename T>
	AverageValues<T>::AverageValues(std::size_t maxValueCount) :
	m_maxValueCount(maxValueCount),
	m_valueSum(0)
	{
	}

	template<typename T>
	T AverageValues<T>::GetAverageValue() const
	{
		if (m_values.empty())
			return 0;

		return m_valueSum / static_cast<T>(m_values.size());
	}

	template<typename T>
	void AverageValues<T>::InsertValue(T value)
	{
		if (m_values.size() >= m_maxValueCount)
		{
			m_valueSum -= m_values.front();
			m_values.erase(m_values.begin());
		}

		m_valueSum += value;
		m_values.emplace_back(std::move(value));
	}
}
