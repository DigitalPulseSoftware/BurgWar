// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/PlayerCommandStore.hpp>
#include <Shared/MatchClientSession.hpp>
#include <Shared/Protocol/Packets.hpp>

namespace bw
{
	PlayerCommandStore::PlayerCommandStore()
	{
#define IncomingCommand(Type) RegisterIncomingCommand<Packets::Type>(#Type, [](MatchClientSession& session, const Packets::Type& packet) \
{ \
	session.HandleIncomingPacket(packet); \
})
#define OutgoingCommand(Type, Flags, Channel) RegisterOutgoingCommand<Packets::Type>(#Type, Flags, Channel)

		// Incoming commands
		IncomingCommand(Auth);
		IncomingCommand(HelloWorld);
		IncomingCommand(PlayersInput);

		// Outgoing commands
		OutgoingCommand(AuthFailure,    Nz::ENetPacketFlag_Reliable, 0);
		OutgoingCommand(AuthSuccess,    Nz::ENetPacketFlag_Reliable, 0);
		OutgoingCommand(CreateEntities, Nz::ENetPacketFlag_Reliable, 1);
		OutgoingCommand(DeleteEntities, Nz::ENetPacketFlag_Reliable, 1);
		OutgoingCommand(HealthUpdate,   Nz::ENetPacketFlag_Reliable, 0);
		OutgoingCommand(HelloWorld,     Nz::ENetPacketFlag_Reliable, 0);
		OutgoingCommand(MatchData,      Nz::ENetPacketFlag_Reliable, 0);
		OutgoingCommand(MatchState,     0,                           1);
		OutgoingCommand(NetworkStrings, Nz::ENetPacketFlag_Reliable, 0);
		OutgoingCommand(PlayAnimation,  Nz::ENetPacketFlag_Reliable, 0);

#undef IncomingCommand
#undef OutgoingCommand
	}
}
