// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/PlayerCommandStore.hpp>
#include <CoreLib/MatchClientSession.hpp>
#include <CoreLib/Protocol/Packets.hpp>

namespace bw
{
	PlayerCommandStore::PlayerCommandStore()
	{
#define IncomingCommand(Type) RegisterIncomingCommand<Packets::Type>(#Type, [](MatchClientSession& session, Packets::Type&& packet) \
{ \
	session.HandleIncomingPacket(std::move(packet)); \
})
#define OutgoingCommand(Type, Flags, Channel) RegisterOutgoingCommand<Packets::Type>(#Type, Flags, Channel)

		// Incoming commands
		IncomingCommand(Auth);
		IncomingCommand(DownloadClientScriptRequest);
		IncomingCommand(HelloWorld);
		IncomingCommand(PlayersInput);
		IncomingCommand(Ready);

		// Outgoing commands
		OutgoingCommand(AuthFailure,                  Nz::ENetPacketFlag_Reliable, 0);
		OutgoingCommand(AuthSuccess,                  Nz::ENetPacketFlag_Reliable, 0);
		OutgoingCommand(ClientScriptList,             Nz::ENetPacketFlag_Reliable, 0);
		OutgoingCommand(ControlEntity,                Nz::ENetPacketFlag_Reliable, 1);
		OutgoingCommand(CreateEntities,               Nz::ENetPacketFlag_Reliable, 1);
		OutgoingCommand(DeleteEntities,               Nz::ENetPacketFlag_Reliable, 1);
		OutgoingCommand(DownloadClientScriptResponse, Nz::ENetPacketFlag_Reliable, 1);
		OutgoingCommand(EntitiesAnimation,            Nz::ENetPacketFlag_Reliable, 0);
		OutgoingCommand(EntitiesInputs,               Nz::ENetPacketFlag_Reliable, 0);
		OutgoingCommand(HealthUpdate,                 Nz::ENetPacketFlag_Reliable, 0);
		OutgoingCommand(HelloWorld,                   Nz::ENetPacketFlag_Reliable, 0);
		OutgoingCommand(InputTimingCorrection,        Nz::ENetPacketFlag_Unsequenced, 0);
		OutgoingCommand(MatchData,                    Nz::ENetPacketFlag_Reliable, 0);
		OutgoingCommand(MatchState,                   0,                           1);
		OutgoingCommand(NetworkStrings,               Nz::ENetPacketFlag_Reliable, 0);

#undef IncomingCommand
#undef OutgoingCommand
	}
}
