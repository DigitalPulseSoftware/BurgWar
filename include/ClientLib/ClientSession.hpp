// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_CLIENTSESSION_HPP
#define BURGWAR_CLIENTLIB_CLIENTSESSION_HPP

#include <CoreLib/SessionBridge.hpp>
#include <CoreLib/Protocol/NetworkStringStore.hpp>
#include <ClientLib/ClientScriptDownloadManager.hpp>
#include <ClientLib/HttpDownloadManager.hpp>
#include <ClientLib/LocalCommandStore.hpp>
#include <Nazara/Core/Signal.hpp>
#include <Nazara/Network/IpAddress.hpp>
#include <memory>
#include <optional>

namespace bw
{
	class BurgApp;
	class LocalMatch;
	class NetworkReactorManager;
	class VirtualDirectory;

	class ClientSession
	{
		friend class LocalCommandStore;

		public:
			struct ConnectionInfo;
			using MatchFactory = std::function<std::shared_ptr<LocalMatch>(ClientSession& session, const Packets::MatchData& matchData)>;

			inline ClientSession(BurgApp& app, MatchFactory matchFactory, std::string playerName);
			ClientSession(const ClientSession&) = delete;
			ClientSession(ClientSession&&) = delete;
			virtual ~ClientSession();

			bool Connect(std::shared_ptr<SessionBridge> sessionBridge);
			void Disconnect();

			Nz::UInt64 EstimateMatchTime() const;

			inline BurgApp& GetApp();
			inline const BurgApp& GetApp() const;
			inline const ConnectionInfo& GetConnectionInfo() const;
			inline const NetworkStringStore& GetNetworkStringStore() const;

			inline bool IsConnected() const;

			void HandleIncomingPacket(Nz::NetPacket& packet);

			template<typename T> void SendPacket(const T& packet);

			void Update();

			ClientSession& operator=(const ClientSession&) = delete;
			ClientSession& operator=(ClientSession&&) = delete;

			NazaraSignal(OnConnected, ClientSession* /*server*/);
			NazaraSignal(OnConnectionInfoUpdate, ClientSession* /*server*/, const ConnectionInfo& /*info*/);
			NazaraSignal(OnDisconnected, ClientSession* /*server*/);

			struct ConnectionInfo
			{
				Nz::UInt32 ping;
				Nz::UInt64 lastReceiveTime;
			};

		protected:
			inline void UpdateInfo(const ConnectionInfo& connectionInfo);

		private:
			enum class ConnectionState
			{
				WaitingForMatchData,

				DownloadingAssets,
				DownloadingScripts,
				
				Ready
			};

			struct PendingPackets
			{
				std::unique_ptr<Packets::ClientAssetList> assetList;
				std::unique_ptr<Packets::ClientScriptList> scriptList;
				std::unique_ptr<Packets::MatchData> matchData;
			};

			void HandleIncomingPacket(Packets::AuthFailure&& packet);
			void HandleIncomingPacket(Packets::AuthSuccess&& packet);
			void HandleIncomingPacket(Packets::ChatMessage&& packet);
			void HandleIncomingPacket(Packets::ClientAssetList&& packet);
			void HandleIncomingPacket(Packets::ClientScriptList&& packet);
			void HandleIncomingPacket(Packets::ConsoleAnswer&& packet);
			void HandleIncomingPacket(Packets::ControlEntity&& packet);
			void HandleIncomingPacket(Packets::CreateEntities&& packet);
			void HandleIncomingPacket(Packets::DeleteEntities&& packet);
			void HandleIncomingPacket(Packets::DownloadClientScriptResponse&& packet);
			void HandleIncomingPacket(Packets::EntitiesAnimation&& packet);
			void HandleIncomingPacket(Packets::EntitiesInputs&& packet);
			void HandleIncomingPacket(Packets::EntityWeapon&& packet);
			void HandleIncomingPacket(Packets::HealthUpdate&& packet);
			void HandleIncomingPacket(Packets::HelloWorld&& packet);
			void HandleIncomingPacket(Packets::InputTimingCorrection&& packet);
			void HandleIncomingPacket(Packets::MatchData&& packet);
			void HandleIncomingPacket(Packets::MatchState&& packet);
			void HandleIncomingPacket(Packets::NetworkStrings&& packet);
			void HandleIncomingPacket(Packets::PlayerWeapons&& packet);

			void OnSessionConnected();
			void OnSessionDisconnected();
			
			NazaraSlot(SessionBridge, OnConnected, m_onConnectedSlot);
			NazaraSlot(SessionBridge, OnDisconnected, m_onDisconnectedSlot);
			NazaraSlot(SessionBridge, OnIncomingPacket, m_onIncomingPacketSlot);

			std::shared_ptr<LocalMatch> m_localMatch;
			std::shared_ptr<SessionBridge> m_bridge;
			std::shared_ptr<VirtualDirectory> m_scriptDirectory;
			std::string m_playerName;
			BurgApp& m_application;
			std::optional<ClientScriptDownloadManager> m_downloadManager;
			std::optional<HttpDownloadManager> m_httpDownloadManager;
			ConnectionInfo m_connectionInfo;
			ConnectionState m_state;
			LocalCommandStore m_commandStore;
			MatchFactory m_matchFactory;
			NetworkStringStore m_stringStore;
			PendingPackets m_pendingPackets;
			Nz::UInt64 m_deltaTime;
	};
}

#include <ClientLib/ClientSession.inl>

#endif
