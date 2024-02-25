// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_COMMANDSTORE_HPP
#define BURGWAR_CORELIB_COMMANDSTORE_HPP

#include <Nazara/Core/ByteStream.hpp>
#include <Nazara/Network/ENetPacket.hpp>
#include <functional>
#include <type_traits>
#include <vector>

namespace bw
{
	class Logger;

	template<typename Peer>
	class CommandStore
	{
		public:
			using PeerRef = std::conditional_t<std::is_pointer_v<Peer>, Peer, Peer&>;

			struct IncomingCommand;
			struct OutgoingCommand;

			CommandStore(const Logger& logger);
			~CommandStore() = default;

			template<typename T> const IncomingCommand& GetIncomingCommand() const;
			template<typename T> const OutgoingCommand& GetOutgoingCommand() const;

			template<typename T>
			void SerializePacket(Nz::ByteStream& packet, const T& data) const;

			bool UnserializePacket(PeerRef peer, Nz::ByteStream& packet) const;

			using UnserializeFunction = std::function<void(PeerRef peer, Nz::ByteStream& packet)>;

			struct IncomingCommand
			{
				bool enabled = false;
				UnserializeFunction unserialize;
				const char* name;
			};

			struct OutgoingCommand
			{
				bool enabled = false;
				const char* name;
				Nz::ENetPacketFlags flags;
				Nz::UInt8 channelId;
			};

		protected:
			template<typename T, typename CB> void RegisterIncomingCommand(const char* name, CB&& callback);
			template<typename T> void RegisterOutgoingCommand(const char* name, Nz::ENetPacketFlags flags, Nz::UInt8 channelId);

		private:
			using HandleFunction = std::function<void(Nz::ByteArray& packet)>;

			std::vector<IncomingCommand> m_incomingCommands;
			std::vector<OutgoingCommand> m_outgoingCommands;
			const Logger& m_logger;
	};
}

#include <CoreLib/CommandStore.inl>

#endif
