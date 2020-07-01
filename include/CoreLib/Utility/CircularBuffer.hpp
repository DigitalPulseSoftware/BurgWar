// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_CIRCULARBUFFER_HPP
#define BURGWAR_CORELIB_CIRCULARBUFFER_HPP

#include <memory>
#include <type_traits>

namespace bw
{
	template<typename T>
	class CircularBuffer
	{
		public:
			CircularBuffer(std::size_t maxValueCount);
			~CircularBuffer();

			T Dequeue();
			template<typename... Args> void Enqueue(Args&&... args);

			std::size_t GetSize() const;

			bool IsEmpty() const;
			bool IsFull() const;

		private:
			std::size_t CycleIndex(std::size_t index) const;

			using MemStorage = std::aligned_storage_t<sizeof(T), alignof(T)>;

			std::size_t m_capacity;
			std::size_t m_head;
			std::size_t m_tail;
			std::unique_ptr<MemStorage[]> m_values;
	};
}

#include <CoreLib/Utility/CircularBuffer.inl>

#endif
