// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_SERVER_CLIENTSESSION_HPP
#define BURGWAR_SERVER_CLIENTSESSION_HPP

#include <Shared/Protocol/Packets.hpp>

namespace bw
{
	class ClientCommandStore;
	class NetworkReactor;

	class ClientSession
	{
		friend ClientCommandStore;
		friend class SessionManager;

		public:
			inline ClientSession(NetworkReactor& reactor, std::size_t peerId, std::size_t sessionId, ClientCommandStore& commandStore);
			~ClientSession() = default;

			void Disconnect();

			inline std::size_t GetPeerId() const;
			inline std::size_t GetSessionId() const;

			template<typename T> void SendPacket(const T& packet);

		private:
			void HandlePacket(Nz::NetPacket&& packet);

			void HandlePacket(const Packets::HelloWorld& packet);

			ClientCommandStore& m_commandStore;
			NetworkReactor& m_reactor;
			std::size_t m_peerId;
			std::size_t m_sessionId;
	};
}

#include <Server/ClientSession.inl>

#endif