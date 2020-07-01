// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Utility/CircularBuffer.hpp>
#include <Nazara/Core/MemoryHelper.hpp>
#include <cassert>

namespace bw
{
	template<typename T>
	CircularBuffer<T>::CircularBuffer(std::size_t maxValueCount) :
	m_capacity(maxValueCount + 1), //< FIXME: Last index is not used due to CycleIndex
	m_head(0),
	m_tail(0)
	{
		assert(m_capacity > 0);
		m_values = std::make_unique<MemStorage[]>(m_capacity);
	}

	template<typename T>
	CircularBuffer<T>::~CircularBuffer()
	{
		for (std::size_t i = m_head; i < m_tail; i = CycleIndex(i + 1))
		{
			T* object = reinterpret_cast<T*>(&m_values[i]);
			Nz::PlacementDestroy(object);
		}
	}
	
	template<typename T>
	T CircularBuffer<T>::Dequeue()
	{
		assert(!IsEmpty());

		T* object = reinterpret_cast<T*>(&m_values[m_head]);
		m_head = CycleIndex(m_head + 1);

		T retObject(std::move(*object));
		Nz::PlacementDestroy(object);

		return retObject;
	}

	template<typename T>
	template<typename ...Args>
	void CircularBuffer<T>::Enqueue(Args&&... args)
	{
		if (IsFull())
			return;

		T* object = reinterpret_cast<T*>(&m_values[m_tail]);
		m_tail = CycleIndex(m_tail + 1);

		Nz::PlacementNew(object, std::forward<Args>(args)...);
	}

	template<typename T>
	std::size_t CircularBuffer<T>::GetSize() const
	{
		if (m_tail >= m_head)
			return m_tail - m_head;
		else
			return m_capacity - (m_head - m_tail);
	}

	template<typename T>
	bool CircularBuffer<T>::IsEmpty() const
	{
		return m_head == m_tail;
	}

	template<typename T>
	bool CircularBuffer<T>::IsFull() const
	{
		return CycleIndex(m_tail + 1) == m_head;
	}

	template<typename T>
	std::size_t CircularBuffer<T>::CycleIndex(std::size_t index) const
	{
		return index % m_capacity;
	}
}
