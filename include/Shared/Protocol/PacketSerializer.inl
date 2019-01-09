// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/Protocol/PacketSerializer.hpp>
#include <Shared/Protocol/CompressedInteger.hpp>
#include <cassert>
#include <stdexcept>

namespace bw
{
	inline PacketSerializer::PacketSerializer(Nz::NetPacket& packetBuffer, bool isWriting) :
	m_buffer(packetBuffer),
	m_isWriting(isWriting)
	{
	}

	inline void PacketSerializer::Read(void* ptr, std::size_t size)
	{
		if (m_buffer.Read(ptr, size) != size)
			throw std::runtime_error("Failed to read");
	}

	inline bool PacketSerializer::IsWriting() const
	{
		return m_isWriting;
	}

	inline void PacketSerializer::Write(const void* ptr, std::size_t size)
	{
		if (m_buffer.Write(ptr, size) != size)
			throw std::runtime_error("Failed to write");
	}

	template<typename DataType>
	void PacketSerializer::Serialize(DataType& data)
	{
		if (!IsWriting())
			m_buffer >> data;
		else
			m_buffer << data;
	}

	template<typename DataType>
	void PacketSerializer::Serialize(const DataType& data) const
	{
		assert(IsWriting());

		m_buffer << data;
	}

	template<typename PacketType, typename DataType>
	void PacketSerializer::Serialize(DataType& data)
	{
		if (!IsWriting())
		{
			PacketType packetData;
			m_buffer >> packetData;

			data = static_cast<DataType>(packetData);
		}
		else
			m_buffer << static_cast<PacketType>(data);
	}

	template<typename PacketType, typename DataType>
	void PacketSerializer::Serialize(const DataType& data) const
	{
		assert(IsWriting());

		m_buffer << static_cast<PacketType>(data);
	}

	template<typename T>
	void PacketSerializer::SerializeArraySize(T& array)
	{
		CompressedUnsigned<Nz::UInt32> arraySize;
		if (IsWriting())
			arraySize = Nz::UInt32(array.size());

		Serialize(arraySize);

		if (!IsWriting())
			array.resize(arraySize);
	}

	template<typename DataType>
	void PacketSerializer::operator&=(DataType& data)
	{
		return Serialize(data);
	}

	template<typename DataType>
	void PacketSerializer::operator&=(const DataType& data) const
	{
		return Serialize(data);
	}
}
