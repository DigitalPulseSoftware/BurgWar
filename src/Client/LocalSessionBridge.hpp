// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENT_LOCALPLAYERBRIDGE_HPP
#define BURGWAR_CLIENT_LOCALPLAYERBRIDGE_HPP

#include <Shared/SessionBridge.hpp>

namespace bw
{
	class LocalSessionBridge : public SessionBridge
	{
		public:
			LocalSessionBridge() = default;
			~LocalSessionBridge() = default;

			void Disconnect() override;

			void SendPacket(Nz::UInt8 channelId, Nz::ENetPacketFlags flags, Nz::NetPacket&& packet) override;

		private:
	};
}

#include <Client/LocalSessionBridge.inl>

#endif