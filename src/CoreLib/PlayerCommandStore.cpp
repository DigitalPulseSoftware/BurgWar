// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/PlayerCommandStore.hpp>
#include <CoreLib/MatchClientSession.hpp>
#include <CoreLib/Protocol/Packets.hpp>

namespace bw
{
	PlayerCommandStore::PlayerCommandStore(const Logger& logger) :
	CommandStore(logger)
	{
#define IncomingCommand(Type) RegisterIncomingCommand<Packets::Type>(#Type, [](MatchClientSession& session, Packets::Type&& packet) \
{ \
	session.HandleIncomingPacket(std::move(packet)); \
})
#define OutgoingCommand(Type, Flags, Channel) RegisterOutgoingCommand<Packets::Type>(#Type, Flags, Channel)

		// Incoming commands
		IncomingCommand(Auth);
		IncomingCommand(DownloadClientFileRequest);
		IncomingCommand(PlayerChat);
		IncomingCommand(PlayerConsoleCommand);
		IncomingCommand(PlayersInput);
		IncomingCommand(PlayerSelectWeapon);
		IncomingCommand(Ready);
		IncomingCommand(ScriptPacket);
		IncomingCommand(UpdatePlayerName);

		// Outgoing commands
		OutgoingCommand(AuthFailure,                  Nz::ENetPacketFlag_Reliable,    0);
		OutgoingCommand(AuthSuccess,                  Nz::ENetPacketFlag_Reliable,    0);
		OutgoingCommand(ChatMessage,                  Nz::ENetPacketFlag_Reliable,    0);
		OutgoingCommand(ClientAssetList,              Nz::ENetPacketFlag_Reliable,    0);
		OutgoingCommand(ClientScriptList,             Nz::ENetPacketFlag_Reliable,    0);
		OutgoingCommand(ConsoleAnswer,                Nz::ENetPacketFlag_Reliable,    0);
		OutgoingCommand(ControlEntity,                Nz::ENetPacketFlag_Reliable,    1);
		OutgoingCommand(CreateEntities,               Nz::ENetPacketFlag_Reliable,    1);
		OutgoingCommand(DeleteEntities,               Nz::ENetPacketFlag_Reliable,    1);
		OutgoingCommand(DisableLayer,                 Nz::ENetPacketFlag_Reliable,    1);
		OutgoingCommand(DownloadClientFileFragment,   Nz::ENetPacketFlag_Reliable,    0);
		OutgoingCommand(DownloadClientFileResponse,   Nz::ENetPacketFlag_Reliable,    0);
		OutgoingCommand(EnableLayer,                  Nz::ENetPacketFlag_Reliable,    1);
		OutgoingCommand(EntitiesAnimation,            Nz::ENetPacketFlag_Reliable,    1);
		OutgoingCommand(EntitiesDeath,                Nz::ENetPacketFlag_Reliable,    1);
		OutgoingCommand(EntitiesInputs,               Nz::ENetPacketFlag_Reliable,    1);
		OutgoingCommand(EntitiesScale,                Nz::ENetPacketFlag_Reliable,    1);
		OutgoingCommand(EntityPhysics,                Nz::ENetPacketFlag_Reliable,    1);
		OutgoingCommand(EntityWeapon,                 Nz::ENetPacketFlag_Reliable,    1);
		OutgoingCommand(HealthUpdate,                 Nz::ENetPacketFlag_Reliable,    1);
		OutgoingCommand(InputTimingCorrection,        Nz::ENetPacketFlag_Unsequenced, 0);
		OutgoingCommand(MatchData,                    Nz::ENetPacketFlag_Reliable,    0);
		OutgoingCommand(MatchState,                   0,                              1);
		OutgoingCommand(NetworkStrings,               Nz::ENetPacketFlag_Reliable,    0);
		OutgoingCommand(PlayerControlEntity,          Nz::ENetPacketFlag_Reliable,    1);
		OutgoingCommand(PlayerJoined,                 Nz::ENetPacketFlag_Reliable,    1);
		OutgoingCommand(PlayerLayer,                  Nz::ENetPacketFlag_Reliable,    1);
		OutgoingCommand(PlayerLeaving,                Nz::ENetPacketFlag_Reliable,    1);
		OutgoingCommand(PlayerNameUpdate,             Nz::ENetPacketFlag_Reliable,    1);
		OutgoingCommand(PlayerPingUpdate,             Nz::ENetPacketFlag_Reliable,    1);
		OutgoingCommand(PlayerWeapons,                Nz::ENetPacketFlag_Reliable,    1);
		OutgoingCommand(ScriptPacket,                 Nz::ENetPacketFlag_Reliable,    1);

#undef IncomingCommand
#undef OutgoingCommand
	}
}
