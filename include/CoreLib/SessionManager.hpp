// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SESSIONMANAGER_HPP
#define BURGWAR_CORELIB_SESSIONMANAGER_HPP

#include <CoreLib/Export.hpp>
#include <CoreLib/NetworkReactor.hpp>
#include <Nazara/Core/MemoryPool.hpp>
#include <vector>

namespace bw
{
	class MatchClientSession;
	class MatchSessions;

	class BURGWAR_CORELIB_API SessionManager
	{
		public:
			inline SessionManager(MatchSessions* owner);
			SessionManager(const SessionManager&) = delete;
			SessionManager(SessionManager&&) = delete;
			virtual ~SessionManager();

			inline MatchSessions* GetOwner();

			virtual void Poll() = 0;

			SessionManager& operator=(const SessionManager&) = delete;
			SessionManager& operator=(SessionManager&&) = delete;

		private:
			MatchSessions* m_owner;
	};
}

#include <CoreLib/SessionManager.inl>

#endif
