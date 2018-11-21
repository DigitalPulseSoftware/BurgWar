// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENT_CLIENTSESSION_HPP
#define BURGWAR_CLIENT_CLIENTSESSION_HPP

#include <Shared/Protocol/NetworkStringStore.hpp>
#include <Shared/SessionBridge.hpp>
#include <Nazara/Core/Signal.hpp>
#include <Nazara/Network/IpAddress.hpp>
#include <memory>

namespace bw
{
	class ClientApp;
	class LocalCommandStore;
	class LocalMatch;

	class ClientSession
	{
		friend class LocalCommandStore;

		public:
			struct ConnectionInfo;

			inline ClientSession(ClientApp& app, const LocalCommandStore& commandStore);
			ClientSession(const ClientSession&) = delete;
			ClientSession(ClientSession&&) = delete;
			virtual ~ClientSession();

			bool Connect(const Nz::IpAddress& address);
			bool Connect(const Nz::String& serverHostname, Nz::UInt16 port, Nz::NetProtocol protocol = Nz::NetProtocol_Any);
			void Disconnect();

			Nz::UInt64 EstimateMatchTime() const;

			inline ClientApp& GetApp();
			inline const ClientApp& GetApp() const;
			inline const ConnectionInfo& GetConnectionInfo() const;
			inline const NetworkStringStore& GetNetworkStringStore() const;

			inline bool IsConnected() const;

			void HandleIncomingPacket(Nz::NetPacket& packet);

			template<typename T> void SendPacket(const T& packet);

			ClientSession& operator=(const ClientSession&) = delete;
			ClientSession& operator=(ClientSession&&) = delete;

			NazaraSignal(OnConnectionInfoUpdate, ClientSession* /*server*/, const ConnectionInfo& /*info*/);

			struct ConnectionInfo
			{
				Nz::UInt32 ping;
				Nz::UInt64 lastReceiveTime;
			};

		protected:
			inline void UpdateInfo(const ConnectionInfo& connectionInfo);

		private:
			void HandleIncomingPacket(const Packets::AuthFailure& packet);
			void HandleIncomingPacket(const Packets::AuthSuccess& packet);
			void HandleIncomingPacket(const Packets::CreateEntities& packet);
			void HandleIncomingPacket(const Packets::DeleteEntities& packet);
			void HandleIncomingPacket(const Packets::HealthUpdate& packet);
			void HandleIncomingPacket(const Packets::HelloWorld& packet);
			void HandleIncomingPacket(const Packets::MatchData& packet);
			void HandleIncomingPacket(const Packets::MatchState& packet);
			void HandleIncomingPacket(const Packets::NetworkStrings& packet);
			void HandleIncomingPacket(const Packets::PlayAnimation& packet);
			
			std::shared_ptr<SessionBridge> m_bridge;
			std::shared_ptr<LocalMatch> m_localMatch;
			ClientApp& m_application;
			const LocalCommandStore& m_commandStore;
			NetworkStringStore m_stringStore;
			ConnectionInfo m_connectionInfo;
			Nz::UInt64 m_deltaTime;
			bool m_isConnected;
	};
}

#include <Client/ClientSession.inl>

#endif
