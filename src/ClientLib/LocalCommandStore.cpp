// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/LocalCommandStore.hpp>
#include <ClientLib/ClientSession.hpp>
#include <CoreLib/Protocol/Packets.hpp>

namespace bw
{
	LocalCommandStore::LocalCommandStore()
	{
#define IncomingCommand(Type) RegisterIncomingCommand<Packets::Type>(#Type, [](ClientSession* session, Packets::Type&& packet) \
{ \
	session->HandleIncomingPacket(std::move(packet)); \
})
#define OutgoingCommand(Type, Flags, Channel) RegisterOutgoingCommand<Packets::Type>(#Type, Flags, Channel)

		// Incoming commands
		IncomingCommand(AuthFailure);
		IncomingCommand(AuthSuccess);
		IncomingCommand(ChatMessage);
		IncomingCommand(ClientScriptList);
		IncomingCommand(ControlEntity);
		IncomingCommand(CreateEntities);
		IncomingCommand(DeleteEntities);
		IncomingCommand(DownloadClientScriptResponse);
		IncomingCommand(EntitiesAnimation);
		IncomingCommand(EntitiesInputs);
		IncomingCommand(HealthUpdate);
		IncomingCommand(HelloWorld);
		IncomingCommand(InputTimingCorrection);
		IncomingCommand(MatchData);
		IncomingCommand(MatchState);
		IncomingCommand(NetworkStrings);

		// Outgoing commands
		OutgoingCommand(Auth,                        Nz::ENetPacketFlag_Reliable, 0);
		OutgoingCommand(DownloadClientScriptRequest, Nz::ENetPacketFlag_Reliable, 0);
		OutgoingCommand(HelloWorld,                  Nz::ENetPacketFlag_Reliable, 0);
		OutgoingCommand(NetworkStrings,              Nz::ENetPacketFlag_Reliable, 0);
		OutgoingCommand(PlayerChat,                  Nz::ENetPacketFlag_Reliable, 0);
		OutgoingCommand(PlayersInput,                Nz::ENetPacketFlag_Reliable, 0);
		OutgoingCommand(Ready,                       Nz::ENetPacketFlag_Reliable, 0);

#undef IncomingCommand
#undef OutgoingCommand
	}
}
