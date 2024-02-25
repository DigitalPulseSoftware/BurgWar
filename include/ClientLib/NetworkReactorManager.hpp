// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_NETWORKREACTORMANAGER_HPP
#define BURGWAR_CLIENTLIB_NETWORKREACTORMANAGER_HPP

#include <CoreLib/NetworkReactor.hpp>
#include <ClientLib/Export.hpp>
#include <memory>
#include <vector>

namespace bw
{
	class Logger;
	class NetworkSessionBridge;

	class BURGWAR_CLIENTLIB_API NetworkReactorManager
	{
		public:
			inline NetworkReactorManager(const Logger& logger);
			NetworkReactorManager(const NetworkReactorManager&) = delete;
			NetworkReactorManager(NetworkReactorManager&&) noexcept = default;
			~NetworkReactorManager() = default;

			inline std::size_t AddReactor(std::unique_ptr<NetworkReactor> reactor);
			inline void ClearReactors();

			std::shared_ptr<NetworkSessionBridge> ConnectToServer(const Nz::IpAddress& serverAddress, Nz::UInt32 data);

			inline const std::unique_ptr<NetworkReactor>& GetReactor(std::size_t reactorId);
			inline std::size_t GetReactorCount() const;

			void Update();

			NetworkReactorManager& operator=(const NetworkReactorManager&) = delete;
			NetworkReactorManager& operator=(NetworkReactorManager&&) = delete;

		private:
			void HandlePeerConnection(bool outgoing, std::size_t peerId, Nz::UInt32 data);
			void HandlePeerDisconnection(std::size_t peerId, Nz::UInt32 data);
			void HandlePeerPacket(std::size_t peerId, Nz::ByteArray& packet);

			std::vector<std::unique_ptr<NetworkReactor>> m_reactors;
			std::vector<std::shared_ptr<NetworkSessionBridge>> m_connections;
			const Logger& m_logger;
	};
}

#include <ClientLib/NetworkReactorManager.inl>

#endif
