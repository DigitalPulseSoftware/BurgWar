// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_SERVER_SESSIONMANAGER_HPP
#define BURGWAR_SERVER_SESSIONMANAGER_HPP

#include <Shared/NetworkReactor.hpp>
#include <Shared/PlayerCommandStore.hpp>
#include <Shared/SessionBridge.hpp>
#include <Shared/SessionManager.hpp>
#include <Nazara/Core/MemoryPool.hpp>
#include <hopstotch/hopscotch_map.h>
#include <vector>

namespace bw
{
	class MatchClientSession;

	class MatchSessions
	{
		public:
			MatchSessions();
			~MatchSessions();

			MatchClientSession* CreateSession(std::unique_ptr<SessionBridge> bridge);
			template<typename T, typename... Args> T* CreateSessionManager(Args&&... args);
			void Deletesession(MatchClientSession* session);

			void Poll();

		private:
			std::size_t m_nextSessionId;
			std::vector<std::unique_ptr<SessionManager>> m_managers;
			PlayerCommandStore m_commandStore;
			Nz::MemoryPool m_sessionPool;
			tsl::hopscotch_map<std::size_t /*sessionId*/, MatchClientSession* /*session*/> m_sessionIdToSession;
	};
}

#include <Shared/MatchSessions.inl>

#endif