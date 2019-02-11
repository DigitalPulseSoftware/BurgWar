// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_SHARED_NETWORKSESSIONBRIDGE_HPP
#define BURGWAR_SHARED_NETWORKSESSIONBRIDGE_HPP

#include <GameLibShared/SessionBridge.hpp>

namespace bw
{
	class NetworkReactor;

	class NetworkSessionBridge : public SessionBridge
	{
		public:
			inline NetworkSessionBridge(NetworkReactor& reactor, std::size_t peerId);
			~NetworkSessionBridge();

			void Disconnect() override;

			inline std::size_t GetPeerId() const;

			void SendPacket(Nz::UInt8 channelId, Nz::ENetPacketFlags flags, Nz::NetPacket&& packet) override;

		private:
			std::size_t m_peerId;
			NetworkReactor& m_reactor;
	};
}

#include <GameLibShared/NetworkSessionBridge.inl>

#endif