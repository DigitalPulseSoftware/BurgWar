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

	class ServerConnection
	{
		friend BurgApp;

		public:
			struct ConnectionInfo;

			inline ServerConnection(BurgApp& application, const ServerCommandStore& commandStore);
			ServerConnection(const ServerConnection&) = delete;
			ServerConnection(ServerConnection&&) = delete;
			~ServerConnection() = default;

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

			ServerConnection& operator=(const ServerConnection&) = delete;
			ServerConnection& operator=(ServerConnection&&) = delete;

			NazaraSignal(OnConnected,            ServerConnection* /*server*/, Nz::UInt32 /*data*/);
			NazaraSignal(OnConnectionInfoUpdate, ServerConnection* /*server*/, const ConnectionInfo& /*info*/);
			NazaraSignal(OnDisconnected,         ServerConnection* /*server*/, Nz::UInt32 /*data*/);

			// Packet reception signals
			NazaraSignal(OnHelloWorld,                ServerConnection* /*server*/, const Packets::HelloWorld&                /*data*/);
			NazaraSignal(OnNetworkStrings,            ServerConnection* /*server*/, const Packets::NetworkStrings&            /*data*/);

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

			void UpdateNetworkStrings(ServerConnection* server, const Packets::NetworkStrings& data);

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

#include <Client/ServerConnection.inl>

#endif
