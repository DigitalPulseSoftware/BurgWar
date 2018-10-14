// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENT_SERVERCONNECTION_HPP
#define BURGWAR_CLIENT_SERVERCONNECTION_HPP

#include <Shared/Protocol/NetworkStringStore.hpp>
#include <Shared/Protocol/Packets.hpp>
#include <Client/ServerCommandStore.hpp>
#include <Nazara/Core/Signal.hpp>
#include <Nazara/Core/String.hpp>

namespace bw
{
	class BurgApp;
	class NetworkReactor;
	class ServerCommandStore;

	class NetworkClientSession
	{
		friend BurgApp;

		public:
			struct ConnectionInfo;

			inline NetworkClientSession(BurgApp& application, const ServerCommandStore& commandStore);
			NetworkClientSession(const NetworkClientSession&) = delete;
			NetworkClientSession(NetworkClientSession&&) = delete;
			~NetworkClientSession() = default;

			bool Connect(const Nz::String& serverHostname, Nz::UInt32 data = 0);
			inline void Disconnect(Nz::UInt32 data = 0);

			Nz::UInt64 EstimateServerTime() const;

			inline BurgApp& GetApp();
			inline const BurgApp& GetApp() const;
			inline const ConnectionInfo& GetConnectionInfo() const;
			inline const NetworkStringStore& GetNetworkStringStore() const;
			inline std::size_t GetPeerId() const;

			inline bool IsConnected() const;

			inline void RefreshInfos();

			template<typename T> void SendPacket(const T& packet);

			inline void UpdateServerTimeDelta(Nz::UInt64 deltaTime);

			NetworkClientSession& operator=(const NetworkClientSession&) = delete;
			NetworkClientSession& operator=(NetworkClientSession&&) = delete;

			NazaraSignal(OnConnected,            NetworkClientSession* /*server*/, Nz::UInt32 /*data*/);
			NazaraSignal(OnConnectionInfoUpdate, NetworkClientSession* /*server*/, const ConnectionInfo& /*info*/);
			NazaraSignal(OnDisconnected,         NetworkClientSession* /*server*/, Nz::UInt32 /*data*/);

			// Packet reception signals
			NazaraSignal(OnHelloWorld,                NetworkClientSession* /*server*/, const Packets::HelloWorld&                /*data*/);
			NazaraSignal(OnNetworkStrings,            NetworkClientSession* /*server*/, const Packets::NetworkStrings&            /*data*/);

			struct ConnectionInfo
			{
				Nz::UInt32 ping;
				Nz::UInt64 lastReceiveTime;
			};

		private:
			inline void DispatchIncomingPacket(Nz::NetPacket&& packet);
			inline void NotifyConnected(Nz::UInt32 data);
			inline void NotifyDisconnected(Nz::UInt32 data);
			inline void UpdateInfo(const ConnectionInfo& connectionInfo);

			void UpdateNetworkStrings(NetworkClientSession* server, const Packets::NetworkStrings& data);

			BurgApp& m_application;
			const ServerCommandStore& m_commandStore;
			NetworkStringStore m_stringStore;
			NetworkReactor* m_networkReactor;
			ConnectionInfo m_connectionInfo;
			Nz::UInt64 m_deltaTime;
			std::size_t m_peerId;
			bool m_connected;
	};
}

#include <Client/NetworkClientSession.inl>

#endif
