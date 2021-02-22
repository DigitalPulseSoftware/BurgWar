// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_NETWORK_PACKETSERIALIZER_HPP
#define BURGWAR_CORELIB_NETWORK_PACKETSERIALIZER_HPP

#include <CoreLib/Export.hpp>
#include <Nazara/Core/ByteStream.hpp>
#include <vector>

namespace bw
{
	class BURGWAR_CORELIB_API PacketSerializer
	{
		public:
			inline PacketSerializer(Nz::ByteStream& packetBuffer, bool isWriting);
			~PacketSerializer() = default;

			inline void Read(void* ptr, std::size_t size);

			inline bool IsWriting() const;

			inline void Write(const void* ptr, std::size_t size);

			template<typename DataType> void Serialize(DataType& data);
			template<typename DataType> void Serialize(std::vector<DataType>& dataVec);
			template<typename DataType> void Serialize(const DataType& data) const;
			template<typename PacketType, typename DataType> void Serialize(DataType& data);
			template<typename PacketType, typename DataType> void Serialize(const DataType& data) const;

			template<typename T> void SerializeArraySize(T& array);
			template<typename T> void SerializeArraySize(const T& array);

			template<typename E, typename UT = std::underlying_type_t<E>> void SerializeEnum(E& enumValue);

			template<typename DataType> void operator&=(DataType& data);
			template<typename DataType> void operator&=(const DataType& data) const;

		private:
			Nz::ByteStream& m_buffer;
			bool m_isWriting;
	};
}

#include <CoreLib/Protocol/PacketSerializer.inl>

#endif
