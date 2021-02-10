// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#if !defined(BURGWAR_EVENT)
#error You must define BURGWAR_EVENT before including this file
#endif

#ifndef BURGWAR_EVENT_LAST
#define BURGWAR_EVENT_LAST(X) BURGWAR_EVENT(X)
#endif

// Shared gamemode events
BURGWAR_EVENT(Init)
BURGWAR_EVENT(PlayerControlledEntityUpdate)
BURGWAR_EVENT(PlayerJoined)
BURGWAR_EVENT(PlayerLeave)
BURGWAR_EVENT(PlayerNameUpdate)
BURGWAR_EVENT(Tick)

// Server gamemode events
BURGWAR_EVENT(PlayerChat)
BURGWAR_EVENT(PlayerConnected)
BURGWAR_EVENT(PlayerDeath)
BURGWAR_EVENT(PlayerLayerUpdate)
//BURGWAR_EVENT(PlayerSpawn)

// Client gamemode events
BURGWAR_EVENT(ChangeLayer)
BURGWAR_EVENT(Frame)
BURGWAR_EVENT(InitScoreboard)
BURGWAR_EVENT(PlayerPingUpdate)
BURGWAR_EVENT_LAST(PostFrame)

#undef BURGWAR_EVENT
#undef BURGWAR_EVENT_LAST
