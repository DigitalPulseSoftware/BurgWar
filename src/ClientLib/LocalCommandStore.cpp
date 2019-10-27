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
	session->On##Type(session, packet); \
})
#define OutgoingCommand(Type, Flags, Channel) RegisterOutgoingCommand<Packets::Type>(#Type, Flags, Channel)

		// Incoming commands
		IncomingCommand(AuthFailure);
		IncomingCommand(AuthSuccess);
		IncomingCommand(ChatMessage);
		IncomingCommand(ClientAssetList);
		IncomingCommand(ClientScriptList);
		IncomingCommand(ConsoleAnswer);
		IncomingCommand(ControlEntity);
		IncomingCommand(CreateEntities);
		IncomingCommand(DeleteEntities);
		IncomingCommand(DownloadClientScriptResponse);
		IncomingCommand(EntitiesAnimation);
		IncomingCommand(EntitiesDeath);
		IncomingCommand(EntitiesInputs);
		IncomingCommand(EntityWeapon);
		IncomingCommand(HealthUpdate);
		IncomingCommand(InputTimingCorrection);
		IncomingCommand(MatchData);
		IncomingCommand(MatchState);
		IncomingCommand(NetworkStrings);
		IncomingCommand(PlayerLayer);
		IncomingCommand(PlayerWeapons);

		// Outgoing commands
		OutgoingCommand(Auth,                        Nz::ENetPacketFlag_Reliable, 0);
		OutgoingCommand(DownloadClientScriptRequest, Nz::ENetPacketFlag_Reliable, 0);
		OutgoingCommand(NetworkStrings,              Nz::ENetPacketFlag_Reliable, 0);
		OutgoingCommand(PlayerChat,                  Nz::ENetPacketFlag_Reliable, 0);
		OutgoingCommand(PlayerConsoleCommand,        Nz::ENetPacketFlag_Reliable, 1);
		OutgoingCommand(PlayersInput,                Nz::ENetPacketFlag_Reliable, 0);
		OutgoingCommand(PlayerSelectWeapon,          Nz::ENetPacketFlag_Reliable, 0);
		OutgoingCommand(Ready,                       Nz::ENetPacketFlag_Reliable, 0);

#undef IncomingCommand
#undef OutgoingCommand
	}
}
