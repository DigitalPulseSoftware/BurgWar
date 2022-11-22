// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/MatchSessions.hpp>
#include <CoreLib/LogSystem/Logger.hpp>
#include <CoreLib/Match.hpp>
#include <CoreLib/MatchClientSession.hpp>

namespace bw
{
	MatchSessions::MatchSessions(Match& match) :
	m_match(match),
	m_commandStore(m_match.GetLogger()),
	m_sessionPool(1024)
	{
	}

	void MatchSessions::Clear()
	{
		m_sessionPool.Clear();
	}

	void MatchSessions::Poll()
	{
		for (auto& sessionManager : m_managers)
			sessionManager->Poll();
	}

	MatchClientSession* MatchSessions::CreateSession(std::shared_ptr<SessionBridge> bridge)
	{
		std::size_t allocationIndex;
		MatchClientSession* session = m_sessionPool.Allocate(allocationIndex, m_match, m_commandStore, std::move(bridge));

		bwLog(m_match.GetLogger(), LogLevel::Info, "Created session #{0}", allocationIndex);

		return session;
	}

	void MatchSessions::DeleteSession(MatchClientSession* session)
	{
		std::size_t allocationIndex = m_sessionPool.RetrieveEntryIndex(session);
		m_sessionPool.Free(allocationIndex);

		bwLog(m_match.GetLogger(), LogLevel::Info, "Deleted session #{0}", allocationIndex);
	}
}
