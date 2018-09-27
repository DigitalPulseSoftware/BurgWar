// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENT_BURGAPP_HPP
#define BURGWAR_CLIENT_BURGAPP_HPP

#include <Shared/NetworkReactor.hpp>
#include <Client/ServerCommandStore.hpp>
#include <NDK/Application.hpp>
#include <vector>

namespace bw
{
	class NetworkReactor;
	class ServerConnection;

	class BurgApp : public Ndk::Application
	{
		friend ServerConnection;

		public:
			BurgApp(int argc, char* argv[]);
			~BurgApp() = default;

			inline Nz::UInt64 GetAppTime() const;
			inline const ServerCommandStore& GetCommandStore() const;

			int Run();

		private:
			inline std::size_t AddReactor(std::unique_ptr<NetworkReactor> reactor);
			inline void ClearReactors();
			inline const std::unique_ptr<NetworkReactor>& GetReactor(std::size_t reactorId);
			inline std::size_t GetReactorCount() const;

			bool ConnectNewServer(const Nz::String& serverHostname, Nz::UInt32 data, ServerConnection* connection, std::size_t* peerId, NetworkReactor** peerReactor);

			void HandlePeerConnection(bool outgoing, std::size_t peerId, Nz::UInt32 data);
			void HandlePeerDisconnection(std::size_t peerId, Nz::UInt32 data);
			void HandlePeerInfo(std::size_t peerId, const NetworkReactor::PeerInfo& peerInfo);
			void HandlePeerPacket(std::size_t peerId, Nz::NetPacket&& packet);

			std::vector<std::unique_ptr<NetworkReactor>> m_reactors;
			std::vector<ServerConnection*> m_servers;
			ServerCommandStore m_commandStore;
			Nz::UInt64 m_appTime;
			Nz::UInt64 m_lastTime;
	};
}

#include <Client/BurgApp.inl>

#endif