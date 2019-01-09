// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENT_LOCALPLAYERBRIDGE_HPP
#define BURGWAR_CLIENT_LOCALPLAYERBRIDGE_HPP

#include <Shared/SessionBridge.hpp>

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

#include <Client/LocalSessionBridge.inl>

#endif