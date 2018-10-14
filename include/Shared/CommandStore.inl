// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/CommandStore.hpp>
#include <cassert>
#include <iostream>

namespace bw
{
	template<typename Peer>
	template<typename T>
	const typename CommandStore<Peer>::IncomingCommand& CommandStore<Peer>::GetIncomingCommand() const
	{
		const std::size_t packetId = static_cast<std::size_t>(T::Type);
		assert(m_incomingCommands.size() > packetId);

		const IncomingCommand& command = m_incomingCommands[packetId];
		assert(command.enabled);

		return command;
	}

	template<typename Peer>
	template<typename T>
	const typename CommandStore<Peer>::OutgoingCommand& CommandStore<Peer>::GetOutgoingCommand() const
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

		IncomingCommand newCommand;
		newCommand.enabled = true;
		newCommand.unserialize = [cb = std::forward<CB>(callback)](PeerRef peer, Nz::NetPacket&& packet)
		{
			T data;
			try
			{
				PacketSerializer serializer(packet, false);

				Packets::Serialize(serializer, data);
			}
			catch (const std::exception&)
			{
				std::cerr << "Failed to unserialize packet" << std::endl;
				return false;
			}

			cb(peer, data);
			return true;
		};
		newCommand.name = name;

		if (m_incomingCommands.size() <= packetId)
			m_incomingCommands.resize(packetId + 1);

		m_incomingCommands[packetId] = std::move(newCommand);
	}

	template<typename Peer>
	template<typename T>
	void CommandStore<Peer>::RegisterOutgoingCommand(const char* name, Nz::ENetPacketFlags flags, Nz::UInt8 channelId)
	{
		std::size_t packetId = static_cast<std::size_t>(T::Type);

		OutgoingCommand newCommand;
		newCommand.channelId = channelId;
		newCommand.enabled = true;
		newCommand.flags = flags;
		newCommand.name = name;

		if (m_outgoingCommands.size() <= packetId)
			m_outgoingCommands.resize(packetId + 1);

		m_outgoingCommands[packetId] = std::move(newCommand);
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
	}

	template<typename Peer>
	bool CommandStore<Peer>::UnserializePacket(PeerRef peer, Nz::NetPacket&& packet) const
	{
		Nz::UInt8 opcode;
		try
		{
			packet >> opcode;
		}
		catch (const std::exception&)
		{
			std::cerr << "Failed to unserialize opcode" << std::endl;
			return false;
		}

		if (m_incomingCommands.size() <= opcode || !m_incomingCommands[opcode].enabled)
		{
			/*std::size_t peerId;
			if constexpr (std::is_pointer_v<Peer>)
				peerId = peer->GetPeerId();
			else
				peerId = peer.GetPeerId();

			std::cerr << "Client #" << peerId << " sent invalid opcode" << std::endl;*/
			return false;
		}

		m_incomingCommands[opcode].unserialize(peer, std::move(packet));
		return true;
	}
}
