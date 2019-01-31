// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_SHARED_NETWORK_PACKETSERIALIZER_HPP
#define BURGWAR_SHARED_NETWORK_PACKETSERIALIZER_HPP

#include <Nazara/Network/NetPacket.hpp>

namespace bw
{
	class PacketSerializer
	{
		public:
			inline PacketSerializer(Nz::NetPacket& packetBuffer, bool isWriting);
			~PacketSerializer() = default;

			inline void Read(void* ptr, std::size_t size);

			inline bool IsWriting() const;

			inline void Write(const void* ptr, std::size_t size);

			template<typename DataType> void Serialize(DataType& data);
			template<typename DataType> void Serialize(const DataType& data) const;
			template<typename PacketType, typename DataType> void Serialize(DataType& data);
			template<typename PacketType, typename DataType> void Serialize(const DataType& data) const;

			template<typename T> void SerializeArraySize(T& array);
			template<typename T> void SerializeArraySize(const T& array);

			template<typename DataType> void operator&=(DataType& data);
			template<typename DataType> void operator&=(const DataType& data) const;

		private:
			Nz::NetPacket& m_buffer;
			bool m_isWriting;
	};
}

#include <Shared/Protocol/PacketSerializer.inl>

#endif // EREWHON_SHARED_NETWORK_PACKETSERIALIZER_HPP
