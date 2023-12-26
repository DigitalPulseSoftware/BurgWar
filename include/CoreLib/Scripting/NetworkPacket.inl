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
	m_stream(m_content.get(), Nz::OpenModeFlags(Nz::OpenMode::Write)),
	m_packetName(std::move(packetName))
	{
	}
	
	inline NetworkPacket::NetworkPacket(std::string packetName, const std::vector<Nz::UInt8>& content) :
	m_content(std::make_unique<Nz::ByteArray>(content.data(), content.size())),
	m_stream(m_content.get(), Nz::OpenModeFlags(Nz::OpenMode::Read)),
	m_packetName(std::move(packetName))
	{
	}

	inline IncomingNetworkPacket::IncomingNetworkPacket(const NetworkStringStore& stringStore, const Packets::ScriptPacket& packet) :
	NetworkPacket(stringStore.GetString(packet.nameIndex), packet.content)
	{
	}

	inline bool IncomingNetworkPacket::ReadBoolean()
	{
		bool value;
		m_stream >> value;

		return value;
	}

	inline Nz::Color IncomingNetworkPacket::ReadColor()
	{
		Nz::Color color;
		m_stream >> color.r >> color.g >> color.b >> color.a;

		return color;
	}

	inline double IncomingNetworkPacket::ReadDouble()
	{
		double output;
		m_stream >> output;

		return output;
	}

	inline Nz::Int64 IncomingNetworkPacket::ReadCompressedInteger()
	{
		CompressedSigned<Nz::Int64> output;
		m_stream >> output;

		return output;
	}

	inline Nz::UInt64 IncomingNetworkPacket::ReadCompressedUnsigned()
	{
		CompressedUnsigned<Nz::UInt64> output;
		m_stream >> output;

		return output;
	}

	inline float IncomingNetworkPacket::ReadSingle()
	{
		float output;
		m_stream >> output;

		return output;
	}

	inline std::string IncomingNetworkPacket::ReadString()
	{
		std::string output;
		m_stream >> output;

		return output;
	}

	inline Nz::Vector2f IncomingNetworkPacket::ReadVector2()
	{
		Nz::Vector2f output;
		output.x = ReadSingle();
		output.y = ReadSingle();

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
	
	inline void OutgoingNetworkPacket::WriteBoolean(bool value)
	{
		m_stream << value;
		m_stream.FlushBits(); //< FIXME
	}

	inline void OutgoingNetworkPacket::WriteColor(Nz::Color color)
	{
		m_stream << color.r << color.g << color.b << color.a;
	}

	inline void OutgoingNetworkPacket::WriteCompressedInteger(Nz::Int64 number)
	{
		CompressedSigned<Nz::Int64> input(number);
		m_stream << input;
	}

	inline void OutgoingNetworkPacket::WriteCompressedUnsigned(Nz::UInt64 number)
	{
		CompressedUnsigned<Nz::UInt64> input(number);
		m_stream << input;
	}

	inline void OutgoingNetworkPacket::WriteDouble(double number)
	{
		m_stream << number;
	}

	inline void OutgoingNetworkPacket::WriteSingle(float number)
	{
		m_stream << number;
	}

	inline void OutgoingNetworkPacket::WriteString(const std::string& str)
	{
		m_stream << str;
	}
	
	inline void OutgoingNetworkPacket::WriteVector2(const Nz::Vector2f& vec)
	{
		WriteSingle(vec.x);
		WriteSingle(vec.y);
	}
}
