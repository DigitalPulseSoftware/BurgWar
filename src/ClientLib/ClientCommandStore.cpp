// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/ClientCommandStore.hpp>
#include <ClientLib/ClientSession.hpp>
#include <CoreLib/Protocol/Packets.hpp>

namespace bw
{
	ClientCommandStore::ClientCommandStore(const Logger& logger) :
	CommandStore(logger)
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
		IncomingCommand(DisableLayer);
		IncomingCommand(DownloadClientFileFragment);
		IncomingCommand(DownloadClientFileResponse);
		IncomingCommand(EnableLayer);
		IncomingCommand(EntitiesAnimation);
		IncomingCommand(EntitiesDeath);
		IncomingCommand(EntitiesInputs);
		IncomingCommand(EntitiesScale);
		IncomingCommand(EntityPhysics);
		IncomingCommand(EntityWeapon);
		IncomingCommand(HealthUpdate);
		IncomingCommand(InputTimingCorrection);
		IncomingCommand(MapReset);
		IncomingCommand(MatchData);
		IncomingCommand(MatchState);
		IncomingCommand(NetworkStrings);
		IncomingCommand(PlayerControlEntity);
		IncomingCommand(PlayerJoined);
		IncomingCommand(PlayerLayer);
		IncomingCommand(PlayerLeaving);
		IncomingCommand(PlayerNameUpdate);
		IncomingCommand(PlayerPingUpdate);
		IncomingCommand(PlayerWeapons);
		IncomingCommand(ScriptPacket);

		// Outgoing commands
		OutgoingCommand(Auth,                        Nz::ENetPacketFlag_Reliable, 0);
		OutgoingCommand(DownloadClientFileRequest,   Nz::ENetPacketFlag_Reliable, 0);
		OutgoingCommand(NetworkStrings,              Nz::ENetPacketFlag_Reliable, 0);
		OutgoingCommand(PlayerChat,                  Nz::ENetPacketFlag_Reliable, 0);
		OutgoingCommand(PlayerConsoleCommand,        Nz::ENetPacketFlag_Reliable, 1);
		OutgoingCommand(PlayersInput,                Nz::ENetPacketFlag_Reliable, 0);
		OutgoingCommand(PlayerSelectWeapon,          Nz::ENetPacketFlag_Reliable, 0);
		OutgoingCommand(Ready,                       Nz::ENetPacketFlag_Reliable, 0);
		OutgoingCommand(ScriptPacket,                Nz::ENetPacketFlag_Reliable, 1);
		OutgoingCommand(UpdatePlayerName,            Nz::ENetPacketFlag_Reliable, 1);

#undef IncomingCommand
#undef OutgoingCommand
	}
}
