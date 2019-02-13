// Copyright (C) 2019 Jérôme Leclercq
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
			inline LocalSessionBridge(LocalSessionManager& sessionManager, std::size_t peerId, bool isServer);
			~LocalSessionBridge() = default;

			void Disconnect() override;

			inline bool IsServer() const;

			void SendPacket(Nz::UInt8 channelId, Nz::ENetPacketFlags flags, Nz::NetPacket&& packet) override;

		private:
			LocalSessionManager& m_sessionManager;
			std::size_t m_peerId;
			bool m_isServer;
	};
}

#include <ClientLib/LocalSessionBridge.inl>

#endif