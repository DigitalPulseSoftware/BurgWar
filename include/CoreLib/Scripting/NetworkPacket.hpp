// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SCRIPTING_NETWORKPACKET_HPP
#define BURGWAR_CORELIB_SCRIPTING_NETWORKPACKET_HPP

#include <CoreLib/Export.hpp>
#include <CoreLib/Protocol/NetworkStringStore.hpp>
#include <CoreLib/Protocol/Packets.hpp>
#include <Nazara/Core/ByteArray.hpp>
#include <Nazara/Core/ByteStream.hpp>
#include <Nazara/Math/Vector2.hpp>
#include <memory>

namespace bw
{
	class BURGWAR_CORELIB_API NetworkPacket
	{
		public:
			inline NetworkPacket(std::string packetName); //< output
			inline NetworkPacket(std::string packetName, const std::vector<Nz::UInt8>& content); //< input
			NetworkPacket(const NetworkPacket&) = delete;
			NetworkPacket(NetworkPacket&&) = default;
			~NetworkPacket() = default;

			NetworkPacket& operator=(const NetworkPacket&) = delete;
			NetworkPacket& operator=(NetworkPacket&&) = delete;

		protected:
			std::unique_ptr<Nz::ByteArray> m_content;
			Nz::ByteStream m_stream;
			std::string m_packetName;
	};

	class BURGWAR_CORELIB_API IncomingNetworkPacket : public NetworkPacket
	{
		public:
			inline IncomingNetworkPacket(const NetworkStringStore& stringStore, const Packets::ScriptPacket& packet);

			inline bool ReadBoolean();
			inline Nz::Color ReadColor();
			inline double ReadDouble();
			inline Nz::Int64 ReadCompressedInteger();
			inline Nz::UInt64 ReadCompressedUnsigned();
			inline float ReadSingle();
			inline std::string ReadString();
			inline Nz::Vector2f ReadVector2();
	};

	class BURGWAR_CORELIB_API OutgoingNetworkPacket : public NetworkPacket
	{
		public:
			inline OutgoingNetworkPacket(std::string packetName);

			Packets::ScriptPacket ToPacket(const NetworkStringStore& stringStore) const;
			
			inline void WriteBoolean(bool value);
			inline void WriteColor(Nz::Color color);
			inline void WriteCompressedInteger(Nz::Int64 number);
			inline void WriteCompressedUnsigned(Nz::UInt64 number);
			inline void WriteDouble(double number);
			inline void WriteSingle(float number);
			inline void WriteString(const std::string& str);
			inline void WriteVector2(const Nz::Vector2f& vec);
	};
}

#include <CoreLib/Scripting/NetworkPacket.inl>

#endif
