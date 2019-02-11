// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_SHARED_SESSIONMANAGER_HPP
#define BURGWAR_SHARED_SESSIONMANAGER_HPP

#include <GameLibShared/NetworkReactor.hpp>
#include <Nazara/Core/MemoryPool.hpp>
#include <vector>

namespace bw
{
	class MatchClientSession;
	class MatchSessions;

	class SessionManager
	{
		public:
			inline SessionManager(MatchSessions* owner);
			virtual ~SessionManager();

			inline MatchSessions* GetOwner();

			virtual void Poll() = 0;

		private:
			MatchSessions* m_owner;
	};
}

#include <GameLibShared/SessionManager.inl>

#endif