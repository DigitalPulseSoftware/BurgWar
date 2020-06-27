// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_LOCALPLAYERBRIDGE_HPP
#define BURGWAR_CLIENTLIB_LOCALPLAYERBRIDGE_HPP

#include <CoreLib/SessionBridge.hpp>

namespace bw
{
	class LocalSessionManager;

	class LocalSessionBridge : public SessionBridge
	{
		public:
			LocalSessionBridge(LocalSessionManager& sessionManager, std::size_t peerId, bool isServer);
			~LocalSessionBridge() = default;

			void Disconnect() override;

			void HandleIncomingPacket(Nz::NetPacket& packet) override;
			inline bool IsServer() const;

			void QueryInfo(std::function<void(const SessionInfo& info)> callback) const override;

			void SendPacket(Nz::UInt8 channelId, Nz::ENetPacketFlags flags, Nz::NetPacket&& packet) override;

		private:
			std::size_t m_peerId;
			Nz::UInt64 m_lastReceiveTime;
			mutable SessionInfo m_sessionInfo;
			LocalSessionManager& m_sessionManager;
			bool m_isServer;
	};
}

#include <ClientLib/LocalSessionBridge.inl>

#endif
