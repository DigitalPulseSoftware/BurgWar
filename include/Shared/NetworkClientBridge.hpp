// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_SHARED_NETWORKCLIENTBRIDGE_HPP
#define BURGWAR_SHARED_NETWORKCLIENTBRIDGE_HPP

#include <Shared/SessionBridge.hpp>
#include <Nazara/Core/Signal.hpp>

namespace bw
{
	class NetworkReactor;

	class NetworkClientBridge : public SessionBridge
	{
		public:
			inline NetworkClientBridge(NetworkReactor& reactor, std::size_t peerId);
			~NetworkClientBridge();

			void Disconnect() override;

			inline std::size_t GetPeerId() const;

			void SendPacket(Nz::UInt8 channelId, Nz::ENetPacketFlags flags, Nz::NetPacket&& packet) override;

			NazaraSignal(OnConnected, Nz::UInt32 /*data*/);
			NazaraSignal(OnDisconnected, Nz::UInt32 /*data*/);
			NazaraSignal(OnIncomingPacket, Nz::NetPacket& /*packet*/);

		private:
			std::size_t m_peerId;
			NetworkReactor& m_reactor;
	};
}

#include <Shared/NetworkClientBridge.inl>

#endif