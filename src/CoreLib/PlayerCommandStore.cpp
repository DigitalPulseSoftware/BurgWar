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
		IncomingCommand(PlayerChat);
		IncomingCommand(PlayerConsoleCommand);
		IncomingCommand(PlayersInput);
		IncomingCommand(PlayerSelectWeapon);
		IncomingCommand(Ready);

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
		OutgoingCommand(DownloadClientScriptResponse, Nz::ENetPacketFlag_Reliable,    0);
		OutgoingCommand(EnableLayer,                  Nz::ENetPacketFlag_Reliable,    1);
		OutgoingCommand(EntitiesAnimation,            Nz::ENetPacketFlag_Reliable,    1);
		OutgoingCommand(EntitiesDeath,                Nz::ENetPacketFlag_Reliable,    1);
		OutgoingCommand(EntitiesInputs,               Nz::ENetPacketFlag_Reliable,    1);
		OutgoingCommand(EntityWeapon,                 Nz::ENetPacketFlag_Reliable,    1);
		OutgoingCommand(HealthUpdate,                 Nz::ENetPacketFlag_Reliable,    1);
		OutgoingCommand(InputTimingCorrection,        Nz::ENetPacketFlag_Unsequenced, 0);
		OutgoingCommand(MatchData,                    Nz::ENetPacketFlag_Reliable,    0);
		OutgoingCommand(MatchState,                   0,                              1);
		OutgoingCommand(NetworkStrings,               Nz::ENetPacketFlag_Reliable,    0);
		OutgoingCommand(PlayerLayer,                  Nz::ENetPacketFlag_Reliable,    1);
		OutgoingCommand(PlayerWeapons,                Nz::ENetPacketFlag_Reliable,    1);

#undef IncomingCommand
#undef OutgoingCommand
	}
}
