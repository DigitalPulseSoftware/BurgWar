// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENT_LOCALSESSIONMANAGER_HPP
#define BURGWAR_CLIENT_LOCALSESSIONMANAGER_HPP

#include <Shared/SessionManager.hpp>
#include <Nazara/Core/MemoryPool.hpp>
#include <vector>

namespace bw
{
	class MatchClientSession;
	class MatchSessions;

	class LocalSessionManager : public SessionManager
	{
		public:
			LocalSessionManager(MatchSessions* owner);
			~LocalSessionManager();

			std::size_t CreateSession();

			void Poll() override;

		private:
			std::vector<MatchClientSession*> m_peerIdToSession;
	};
}

#include <Shared/NetworkSessionManager.inl>

#endif