// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/CommandStore.hpp>
#include <CoreLib/LogSystem/Logger.hpp>
#include <CoreLib/Protocol/Packets.hpp>
#include <cassert>

namespace bw
{
	template<typename Peer>
	inline CommandStore<Peer>::CommandStore(const Logger& logger) :
	m_logger(logger)
	{
	}

	template<typename Peer>
	template<typename T>
	auto CommandStore<Peer>::GetIncomingCommand() const -> const IncomingCommand&
	{
		const std::size_t packetId = static_cast<std::size_t>(T::Type);
		assert(m_incomingCommands.size() > packetId);

		const IncomingCommand& command = m_incomingCommands[packetId];
		assert(command.enabled);

		return command;
	}

	template<typename Peer>
	template<typename T>
	auto CommandStore<Peer>::GetOutgoingCommand() const -> const OutgoingCommand&
	{
		const std::size_t packetId = static_cast<std::size_t>(T::Type);
		assert(m_outgoingCommands.size() > packetId);

		const OutgoingCommand& command = m_outgoingCommands[packetId];
		assert(command.enabled);

		return command;
	}

	template<typename Peer>
	template<typename T, typename CB>
	void CommandStore<Peer>::RegisterIncomingCommand(const char* name, CB&& callback)
	{
		std::size_t packetId = static_cast<std::size_t>(T::Type);

		if (m_incomingCommands.size() <= packetId)
			m_incomingCommands.resize(packetId + 1);

		IncomingCommand& newCommand = m_incomingCommands[packetId];
		newCommand.enabled = true;
		newCommand.unserialize = [this, cb = std::forward<CB>(callback)](PeerRef peer, Nz::NetPacket& packet)
		{
			T data;
			try
			{
				PacketSerializer serializer(packet, false);

				Packets::Serialize(serializer, data);
			}
			catch (const std::exception&)
			{
				bwLog(m_logger, LogLevel::Error, "Failed to unserialize packet");
				return false;
			}

			cb(peer, std::move(data));
			return true;
		};
		newCommand.name = name;
	}

	template<typename Peer>
	template<typename T>
	void CommandStore<Peer>::RegisterOutgoingCommand(const char* name, Nz::ENetPacketFlags flags, Nz::UInt8 channelId)
	{
		std::size_t packetId = static_cast<std::size_t>(T::Type);

		if (m_outgoingCommands.size() <= packetId)
			m_outgoingCommands.resize(packetId + 1);

		OutgoingCommand& newCommand = m_outgoingCommands[packetId];
		newCommand.channelId = channelId;
		newCommand.enabled = true;
		newCommand.flags = flags;
		newCommand.name = name;
	}

	template<typename Peer>
	template<typename T>
	void CommandStore<Peer>::SerializePacket(Nz::NetPacket& packet, const T& data) const
	{
		packet << static_cast<Nz::UInt8>(T::Type);

		// We need to cast the const away because our serialize functions require a non-const reference as they performs both reading and writing
		// If you have a better idea...
		T& dataRef = const_cast<T&>(data);

		PacketSerializer serializer(packet, true);
		Packets::Serialize(serializer, dataRef);

		packet.FlushBits();
	}

	template<typename Peer>
	bool CommandStore<Peer>::UnserializePacket(PeerRef peer, Nz::NetPacket& packet) const
	{
		Nz::UInt8 opcode;
		try
		{
			packet >> opcode;
		}
		catch (const std::exception&)
		{
			bwLog(m_logger, LogLevel::Error, "Failed to unserialize opcode");
			return false;
		}

		if (m_incomingCommands.size() <= opcode || !m_incomingCommands[opcode].enabled)
		{
			bwLog(m_logger, LogLevel::Error, "Client :derp: sent invalid or disabled opcode: {}", +opcode);
			return false;
		}

		m_incomingCommands[opcode].unserialize(peer, packet);
		return true;
	}
}
