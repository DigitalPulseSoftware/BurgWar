// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_NETWORK_REACTOR_HPP
#define BURGWAR_CORELIB_NETWORK_REACTOR_HPP

#include <CoreLib/Export.hpp>
#include <Nazara/Core/Time.hpp>
#include <Nazara/Network/ENetHost.hpp>
#include <concurrentqueue/concurrentqueue.h>
#include <atomic>
#include <functional>
#include <thread>
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

	class BURGWAR_CORELIB_API NetworkReactor
	{
		public:
			struct PeerInfo;
			using PeerInfoCallback = std::function<void(PeerInfo& peerInfo)>;

			NetworkReactor(std::size_t firstId, Nz::NetProtocol protocol, Nz::UInt16 port, std::size_t maxClient);
			NetworkReactor(const NetworkReactor&) = delete;
			NetworkReactor(NetworkReactor&&) = delete;
			~NetworkReactor();

			std::size_t ConnectTo(Nz::IpAddress address, Nz::UInt32 data = 0);
			void DisconnectPeer(std::size_t peerId, Nz::UInt32 data = 0, DisconnectionType type = DisconnectionType::Normal);

			template<typename ConnectCB, typename DisconnectCB, typename DataCB>
			void Poll(ConnectCB&& onConnection, DisconnectCB&& onDisconnection, DataCB&& onData);

			inline Nz::NetProtocol GetProtocol() const;

			void QueryInfo(std::size_t peerId, PeerInfoCallback callback);

			void SendData(std::size_t peerId, Nz::UInt8 channelId, Nz::ENetPacketFlags flags, Nz::NetPacket&& packet);

			NetworkReactor& operator=(const NetworkReactor&) = delete;
			NetworkReactor& operator=(NetworkReactor&&) = delete;

			struct PeerInfo
			{
				Nz::Time timeSinceLastReceive;
				Nz::UInt32 ping;
				Nz::UInt32 totalPacketLost;
				Nz::UInt32 totalPacketReceived;
				Nz::UInt32 totalPacketSent;
				Nz::UInt64 totalByteReceived;
				Nz::UInt64 totalByteSent;
			};

			static constexpr std::size_t InvalidPeerId = std::numeric_limits<std::size_t>::max();
	
		private:
			void EnsureProperDisconnection(const moodycamel::ProducerToken& producterToken, moodycamel::ConsumerToken& token);
			void HandleConnectionRequests(moodycamel::ConsumerToken& token);
			void ReceivePackets(const moodycamel::ProducerToken& producterToken);
			void SendPackets(const moodycamel::ProducerToken& producterToken, moodycamel::ConsumerToken& token);
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

				struct PeerInfoResponse
				{
					PeerInfo peerInfo;
					PeerInfoCallback callback;
				};

				std::size_t peerId = InvalidPeerId;
				std::variant<ConnectEvent, DisconnectEvent, PacketEvent, PeerInfoResponse> data;
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

				struct QueryPeerInfo 
				{
					PeerInfoCallback callback;
				};

				std::size_t peerId = InvalidPeerId;
				std::variant<DisconnectEvent, PacketEvent, QueryPeerInfo> data;
			};

			std::atomic_bool m_running;
			std::size_t m_firstId;
			std::thread m_thread;
			std::vector<Nz::ENetPeer*> m_clients;
			moodycamel::ConcurrentQueue<ConnectionRequest> m_connectionRequests;
			moodycamel::ConcurrentQueue<IncomingEvent> m_incomingQueue;
			moodycamel::ConcurrentQueue<OutgoingEvent> m_outgoingQueue;
			Nz::ENetHost m_host;
			Nz::NetProtocol m_protocol;
	};
}

#include <CoreLib/NetworkReactor.inl>

#endif
