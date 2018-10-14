// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENT_CLIENTSESSION_HPP
#define BURGWAR_CLIENT_CLIENTSESSION_HPP

#include <Shared/Protocol/NetworkStringStore.hpp>
#include <Nazara/Core/Signal.hpp>

namespace bw
{
	class BurgApp;
	class ServerCommandStore;

	class ClientSession
	{
		public:
			struct ConnectionInfo;

			inline ClientSession(BurgApp& app, const ServerCommandStore& commandStore);
			ClientSession(const ClientSession&) = delete;
			ClientSession(ClientSession&&) = delete;
			virtual ~ClientSession();

			Nz::UInt64 EstimateMatchTime() const;

			inline BurgApp& GetApp();
			inline const BurgApp& GetApp() const;
			inline const ConnectionInfo& GetConnectionInfo() const;
			inline const NetworkStringStore& GetNetworkStringStore() const;

			void HandleIncomingPacket(Nz::NetPacket&& packet);

			virtual void RefreshInfos() = 0;

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
			inline void DispatchIncomingPacket(Nz::NetPacket&& packet);
			inline void UpdateInfo(const ConnectionInfo& connectionInfo);

		private:
			BurgApp& m_application;
			const ServerCommandStore& m_commandStore;
			NetworkStringStore m_stringStore;
			ConnectionInfo m_connectionInfo;
			Nz::UInt64 m_deltaTime;
	};
}

#include <Client/ClientSession.inl>

#endif
