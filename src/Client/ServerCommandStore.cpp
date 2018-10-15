// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/ServerCommandStore.hpp>
#include <Client/ClientSession.hpp>
#include <Shared/Protocol/Packets.hpp>

namespace bw
{
	ServerCommandStore::ServerCommandStore()
	{
#define IncomingCommand(Type) RegisterIncomingCommand<Packets::Type>(#Type, [](ClientSession* session, const Packets::Type& packet) \
{ \
	session->HandleIncomingPacket(packet); \
})
#define OutgoingCommand(Type, Flags, Channel) RegisterOutgoingCommand<Packets::Type>(#Type, Flags, Channel)

		// Incoming commands
		IncomingCommand(HelloWorld);

		// Outgoing commands
		OutgoingCommand(HelloWorld, Nz::ENetPacketFlag_Reliable, 0);
		OutgoingCommand(NetworkStrings, Nz::ENetPacketFlag_Reliable, 0);

#undef IncomingCommand
#undef OutgoingCommand
	}
}
