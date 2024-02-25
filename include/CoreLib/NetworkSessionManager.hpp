// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_SERVER_NETWORKSESSIONMANAGER_HPP
#define BURGWAR_SERVER_NETWORKSESSIONMANAGER_HPP

#include <CoreLib/Export.hpp>
#include <CoreLib/NetworkReactor.hpp>
#include <CoreLib/SessionManager.hpp>
#include <NazaraUtils/MemoryPool.hpp>
#include <vector>

namespace bw
{
	class MatchClientSession;
	class MatchSessions;

	class BURGWAR_CORELIB_API NetworkSessionManager : public SessionManager
	{
		public:
			NetworkSessionManager(MatchSessions* owner, Nz::UInt16 port, std::size_t maxClient);
			~NetworkSessionManager();

			void Poll() override;

		private:
			void HandlePeerConnection(bool outgoing, std::size_t peerId, Nz::UInt32 data);
			void HandlePeerDisconnection(std::size_t peerId, Nz::UInt32 data);
			void HandlePeerPacket(std::size_t peerId, Nz::ByteArray&& packet);

			std::vector<MatchClientSession*> m_peerIdToSession;
			NetworkReactor m_reactor;
	};
}

#include <CoreLib/NetworkSessionManager.inl>

#endif
