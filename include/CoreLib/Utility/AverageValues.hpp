// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_AVERAGEVALUES_HPP
#define BURGWAR_CORELIB_AVERAGEVALUES_HPP

#include <vector>

namespace bw
{
	template<typename T>
	class AverageValues
	{
		public:
			AverageValues(std::size_t maxValueCount);
			~AverageValues() = default;

			T GetAverageValue() const;

			void InsertValue(T value);

		private:
			std::vector<T> m_values;
			std::size_t m_maxValueCount;
			T m_valueSum;
	};
}

#include <CoreLib/Utility/AverageValues.inl>

#endif
