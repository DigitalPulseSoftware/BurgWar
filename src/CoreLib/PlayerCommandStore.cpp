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
		OutgoingCommand(AuthFailure,                  Nz::ENetPacketFlag::Reliable,    0);
		OutgoingCommand(AuthSuccess,                  Nz::ENetPacketFlag::Reliable,    0);
		OutgoingCommand(ChatMessage,                  Nz::ENetPacketFlag::Reliable,    0);
		OutgoingCommand(ClientAssetList,              Nz::ENetPacketFlag::Reliable,    0);
		OutgoingCommand(ClientScriptList,             Nz::ENetPacketFlag::Reliable,    0);
		OutgoingCommand(ConsoleAnswer,                Nz::ENetPacketFlag::Reliable,    0);
		OutgoingCommand(ControlEntity,                Nz::ENetPacketFlag::Reliable,    1);
		OutgoingCommand(CreateEntities,               Nz::ENetPacketFlag::Reliable,    1);
		OutgoingCommand(DeleteEntities,               Nz::ENetPacketFlag::Reliable,    1);
		OutgoingCommand(DisableLayer,                 Nz::ENetPacketFlag::Reliable,    1);
		OutgoingCommand(DownloadClientFileFragment,   Nz::ENetPacketFlag::Reliable,    0);
		OutgoingCommand(DownloadClientFileResponse,   Nz::ENetPacketFlag::Reliable,    0);
		OutgoingCommand(EnableLayer,                  Nz::ENetPacketFlag::Reliable,    1);
		OutgoingCommand(EntitiesAnimation,            Nz::ENetPacketFlag::Reliable,    1);
		OutgoingCommand(EntitiesDeath,                Nz::ENetPacketFlag::Reliable,    1);
		OutgoingCommand(EntitiesInputs,               Nz::ENetPacketFlag::Reliable,    1);
		OutgoingCommand(EntitiesScale,                Nz::ENetPacketFlag::Reliable,    1);
		OutgoingCommand(EntityPhysics,                Nz::ENetPacketFlag::Reliable,    1);
		OutgoingCommand(EntityWeapon,                 Nz::ENetPacketFlag::Reliable,    1);
		OutgoingCommand(HealthUpdate,                 Nz::ENetPacketFlag::Reliable,    1);
		OutgoingCommand(InputTimingCorrection,        Nz::ENetPacketFlag::Unsequenced, 0);
		OutgoingCommand(MapReset,                     Nz::ENetPacketFlag::Reliable,    1);
		OutgoingCommand(MatchData,                    Nz::ENetPacketFlag::Reliable,    0);
		OutgoingCommand(MatchState,                   0,                              1);
		OutgoingCommand(NetworkStrings,               Nz::ENetPacketFlag::Reliable,    0);
		OutgoingCommand(PlayerControlEntity,          Nz::ENetPacketFlag::Reliable,    1);
		OutgoingCommand(PlayerJoined,                 Nz::ENetPacketFlag::Reliable,    1);
		OutgoingCommand(PlayerLayer,                  Nz::ENetPacketFlag::Reliable,    1);
		OutgoingCommand(PlayerLeaving,                Nz::ENetPacketFlag::Reliable,    1);
		OutgoingCommand(PlayerNameUpdate,             Nz::ENetPacketFlag::Reliable,    1);
		OutgoingCommand(PlayerPingUpdate,             Nz::ENetPacketFlag::Reliable,    1);
		OutgoingCommand(PlayerWeapons,                Nz::ENetPacketFlag::Reliable,    1);
		OutgoingCommand(ScriptPacket,                 Nz::ENetPacketFlag::Reliable,    1);

#undef IncomingCommand
#undef OutgoingCommand
	}
}
