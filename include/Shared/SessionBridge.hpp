// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_SHARED_SESSIONBRIDGE_HPP
#define BURGWAR_SHARED_SESSIONBRIDGE_HPP

#include <Shared/PlayerCommandStore.hpp>
#include <Nazara/Core/Signal.hpp>

namespace bw
{
	class MatchClientSession;

	class SessionBridge
	{
		public:
			inline SessionBridge(MatchClientSession* session);
			virtual ~SessionBridge();

			virtual void Disconnect() = 0;

			inline MatchClientSession* GetSession();

			virtual void SendPacket(Nz::UInt8 channelId, Nz::ENetPacketFlags flags, Nz::NetPacket&& data) = 0;

			NazaraSignal(OnConnected, Nz::UInt32 /*data*/);
			NazaraSignal(OnDisconnected, Nz::UInt32 /*data*/);
			NazaraSignal(OnIncomingPacket, Nz::NetPacket& /*packet*/);

		private:
			MatchClientSession* m_session;
	};
}

#include <Shared/SessionBridge.inl>

#endif
