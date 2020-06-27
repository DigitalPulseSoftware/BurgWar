// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/NetworkPacket.hpp>
#include <CoreLib/Protocol/CompressedInteger.hpp>
#include <cassert>

namespace bw
{
	inline NetworkPacket::NetworkPacket(std::string packetName) :
	m_content(std::make_unique<Nz::ByteArray>()),
	m_stream(m_content.get(), Nz::OpenModeFlags(Nz::OpenMode_WriteOnly)),
	m_packetName(std::move(packetName))
	{
	}
	
	inline NetworkPacket::NetworkPacket(std::string packetName, const std::vector<Nz::UInt8>& content) :
	m_content(std::make_unique<Nz::ByteArray>(content.data(), content.size())),
	m_stream(m_content.get(), Nz::OpenModeFlags(Nz::OpenMode_ReadOnly)),
	m_packetName(std::move(packetName))
	{
	}

	inline IncomingNetworkPacket::IncomingNetworkPacket(const NetworkStringStore& stringStore, const Packets::ScriptPacket& packet) :
	NetworkPacket(stringStore.GetString(packet.nameIndex), packet.content)
	{
	}

	inline Nz::UInt64 IncomingNetworkPacket::ReadCompressedUnsigned()
	{
		CompressedUnsigned<Nz::UInt64> output;
		m_stream >> output;

		return output;
	}

	inline std::string IncomingNetworkPacket::ReadString()
	{
		std::string output;
		m_stream >> output;

		return output;
	}

	inline OutgoingNetworkPacket::OutgoingNetworkPacket(std::string packetName) :
	NetworkPacket(std::move(packetName))
	{
	}

	inline Packets::ScriptPacket OutgoingNetworkPacket::ToPacket(const NetworkStringStore& stringStore) const
	{
		Packets::ScriptPacket packet;
		packet.nameIndex = stringStore.CheckStringIndex(m_packetName);
		packet.content.assign(m_content->begin(), m_content->end());

		return packet;
	}
	
	inline void OutgoingNetworkPacket::WriteCompressedUnsigned(Nz::UInt64 number)
	{
		CompressedUnsigned<Nz::UInt64> input(number);
		m_stream << input;
	}

	inline void OutgoingNetworkPacket::WriteString(const std::string& str)
	{
		m_stream << str;
	}
}
