// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SCRIPTING_NETWORKPACKET_HPP
#define BURGWAR_CORELIB_SCRIPTING_NETWORKPACKET_HPP

#include <CoreLib/Protocol/NetworkStringStore.hpp>
#include <CoreLib/Protocol/Packets.hpp>
#include <Nazara/Core/ByteArray.hpp>
#include <Nazara/Core/ByteStream.hpp>
#include <memory>

namespace bw
{
	class NetworkPacket
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

	class IncomingNetworkPacket : public NetworkPacket
	{
		public:
			inline IncomingNetworkPacket(const NetworkStringStore& stringStore, const Packets::ScriptPacket& packet);

			inline Nz::UInt64 ReadCompressedUnsigned();
			inline std::string ReadString();
	};

	class OutgoingNetworkPacket : public NetworkPacket
	{
		public:
			inline OutgoingNetworkPacket(std::string packetName);

			Packets::ScriptPacket ToPacket(const NetworkStringStore& stringStore) const;
			
			inline void WriteCompressedUnsigned(Nz::UInt64 number);
			inline void WriteString(const std::string& str);
	};
}

#include <CoreLib/Scripting/NetworkPacket.inl>

#endif
