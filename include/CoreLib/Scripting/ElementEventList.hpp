// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#if !defined(BURGWAR_EVENT)
#error You must define BURGWAR_EVENT before including this file
#endif

#ifndef BURGWAR_EVENT_LAST
#define BURGWAR_EVENT_LAST(X) BURGWAR_EVENT(X)
#endif

/*** Common ***/

// Shared element events
BURGWAR_EVENT(CollisionStop)
BURGWAR_EVENT(CollisionStart)
BURGWAR_EVENT(Death)
BURGWAR_EVENT(Destroyed)
BURGWAR_EVENT(Died)
BURGWAR_EVENT(HealthUpdate)
BURGWAR_EVENT(Init)
BURGWAR_EVENT(ScaleUpdate)
BURGWAR_EVENT(TakeDamage)
BURGWAR_EVENT(Tick)

// Client element events
BURGWAR_EVENT(Frame)
BURGWAR_EVENT(PostFrame)

/*** Entities ***/
BURGWAR_EVENT(InputUpdate)

/*** Weapons ***/

// Shared weapon events
BURGWAR_EVENT(Attack)
BURGWAR_EVENT(SwitchOn)
BURGWAR_EVENT(SwitchOff)
BURGWAR_EVENT_LAST(AttackFinish)

#undef BURGWAR_EVENT
#undef BURGWAR_EVENT_LAST
