// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SESSIONMANAGER_HPP
#define BURGWAR_CORELIB_SESSIONMANAGER_HPP

#include <CoreLib/NetworkReactor.hpp>
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

#include <CoreLib/SessionManager.inl>

#endif
