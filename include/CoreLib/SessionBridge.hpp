// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SESSIONBRIDGE_HPP
#define BURGWAR_CORELIB_SESSIONBRIDGE_HPP

#include <CoreLib/PlayerCommandStore.hpp>
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

			inline bool IsConnected() const;

			inline void HandleConnection(Nz::UInt32 data);
			inline void HandleDisconnection(Nz::UInt32 data);
			inline void HandleIncomingPacket(Nz::NetPacket& packet);

			virtual void SendPacket(Nz::UInt8 channelId, Nz::ENetPacketFlags flags, Nz::NetPacket&& data) = 0;

			NazaraSignal(OnConnected, Nz::UInt32 /*data*/);
			NazaraSignal(OnDisconnected, Nz::UInt32 /*data*/);
			NazaraSignal(OnIncomingPacket, Nz::NetPacket& /*packet*/);

		private:
			MatchClientSession* m_session;
			bool m_isConnected;
	};
}

#include <CoreLib/SessionBridge.inl>

#endif
