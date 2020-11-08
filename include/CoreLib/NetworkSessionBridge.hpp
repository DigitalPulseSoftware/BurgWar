// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_NETWORKSESSIONBRIDGE_HPP
#define BURGWAR_CORELIB_NETWORKSESSIONBRIDGE_HPP

#include <CoreLib/SessionBridge.hpp>

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

			bool IsLocal() const override;

			void QueryInfo(std::function<void(const SessionInfo& info)> callback) const override;

			void SendPacket(Nz::UInt8 channelId, Nz::ENetPacketFlags flags, Nz::NetPacket&& packet) override;

		private:
			std::size_t m_peerId;
			NetworkReactor& m_reactor;
	};
}

#include <CoreLib/NetworkSessionBridge.inl>

#endif
