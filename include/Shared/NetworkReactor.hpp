// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_SHARED_NETWORK_REACTOR_HPP
#define BURGWAR_SHARED_NETWORK_REACTOR_HPP

#include <Nazara/Core/Thread.hpp>
#include <Nazara/Network/ENetHost.hpp>
#include <concurrentqueue/concurrentqueue.h>
#include <atomic>
#include <functional>
#include <variant>
#include <vector>

namespace bw
{
	enum class DisconnectionType
	{
		Kick,   // DisconnectNow
		Later,  // DisconnectLater
		Normal  // Disconnect
	};

	class NetworkReactor
	{
		public:
			struct PeerInfo;

			NetworkReactor(std::size_t firstId, Nz::NetProtocol protocol, Nz::UInt16 port, std::size_t maxClient);
			NetworkReactor(const NetworkReactor&) = delete;
			NetworkReactor(NetworkReactor&&) = delete;
			~NetworkReactor();

			std::size_t ConnectTo(Nz::IpAddress address, Nz::UInt32 data = 0);
			void DisconnectPeer(std::size_t peerId, Nz::UInt32 data = 0, DisconnectionType type = DisconnectionType::Normal);

			template<typename ConnectCB, typename DisconnectCB, typename DataCB, typename InfoCB>
			void Poll(ConnectCB&& onConnection, DisconnectCB&& onDisconnection, DataCB&& onData, InfoCB&& onInfo);

			inline Nz::NetProtocol GetProtocol() const;

			void QueryInfo(std::size_t peerId);

			void SendData(std::size_t peerId, Nz::UInt8 channelId, Nz::ENetPacketFlags flags, Nz::NetPacket&& packet);

			NetworkReactor& operator=(const NetworkReactor&) = delete;
			NetworkReactor& operator=(NetworkReactor&&) = delete;

			struct PeerInfo
			{
				Nz::UInt32 lastReceiveTime;
				Nz::UInt32 ping;
			};

			static constexpr std::size_t InvalidPeerId = std::numeric_limits<std::size_t>::max();
	
		private:
			void HandleConnectionRequests(const moodycamel::ConsumerToken& token);
			void ReceivePackets(const moodycamel::ProducerToken& producterToken);
			void SendPackets(const moodycamel::ProducerToken& producterToken, const moodycamel::ConsumerToken& token);
			void WorkerThread();

			struct ConnectionRequest
			{
				using Callback = std::function<void(std::size_t clientId)>;

				Callback callback;
				Nz::IpAddress remoteAddress;
				Nz::UInt32 data;
			};

			struct IncomingEvent
			{
				struct ConnectEvent
				{
					bool outgoingConnection;
					Nz::UInt32 data;
				};

				struct DisconnectEvent
				{
					Nz::UInt32 data;
				};

				struct PacketEvent
				{
					Nz::NetPacket packet;
				};

				std::size_t peerId = InvalidPeerId;
				std::variant<ConnectEvent, DisconnectEvent, PacketEvent, PeerInfo> data;
			};

			struct OutgoingEvent
			{
				struct DisconnectEvent
				{
					DisconnectionType type;
					Nz::UInt32 data;
				};

				struct PacketEvent
				{
					Nz::ENetPacketFlags flags;
					Nz::UInt8 channelId;
					Nz::NetPacket packet;
				};

				struct QueryPeerInfo {};

				std::size_t peerId = InvalidPeerId;
				std::variant<DisconnectEvent, PacketEvent, QueryPeerInfo> data;
			};

			std::atomic_bool m_running;
			std::size_t m_firstId;
			std::vector<Nz::ENetPeer*> m_clients;
			moodycamel::ConcurrentQueue<ConnectionRequest> m_connectionRequests;
			moodycamel::ConcurrentQueue<IncomingEvent> m_incomingQueue;
			moodycamel::ConcurrentQueue<OutgoingEvent> m_outgoingQueue;
			Nz::ENetHost m_host;
			Nz::NetProtocol m_protocol;
			Nz::Thread m_thread;
	};
}

#include <Shared/NetworkReactor.inl>

#endif
