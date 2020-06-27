// Copyright (C) 2020 Jérôme Leclercq
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
			struct SessionInfo;

			inline SessionBridge(MatchClientSession* session);
			virtual ~SessionBridge();

			virtual void Disconnect() = 0;

			inline MatchClientSession* GetSession();

			inline bool IsConnected() const;

			virtual void HandleConnection(Nz::UInt32 data);
			virtual void HandleDisconnection(Nz::UInt32 data);
			virtual void HandleIncomingPacket(Nz::NetPacket& packet);

			virtual void QueryInfo(std::function<void(const SessionInfo& info)> callback) const = 0;

			virtual void SendPacket(Nz::UInt8 channelId, Nz::ENetPacketFlags flags, Nz::NetPacket&& data) = 0;

			NazaraSignal(OnConnected, Nz::UInt32 /*data*/);
			NazaraSignal(OnDisconnected, Nz::UInt32 /*data*/);
			NazaraSignal(OnIncomingPacket, Nz::NetPacket& /*packet*/);

			struct SessionInfo
			{
				Nz::UInt32 ping;
				Nz::UInt32 timeSinceLastReceive;
				Nz::UInt32 totalPacketReceived;
				Nz::UInt32 totalPacketLost;
				Nz::UInt32 totalPacketSent;
				Nz::UInt64 totalByteReceived;
				Nz::UInt64 totalByteSent;
			};

		private:
			MatchClientSession* m_session;
			bool m_isConnected;
	};
}

#include <CoreLib/SessionBridge.inl>

#endif
