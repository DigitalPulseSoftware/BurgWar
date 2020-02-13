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
	m_nextSessionId(0),
	m_match(match),
	m_commandStore(m_match.GetLogger()),
	m_sessionPool(sizeof(MatchClientSession))
	{
	}

	MatchSessions::~MatchSessions()
	{
		Clear();
	}

	void MatchSessions::Clear()
	{
		for (const auto& pair : m_sessionIdToSession)
			m_sessionPool.Delete(pair.second);

		m_sessionIdToSession.clear();
	}

	void MatchSessions::Poll()
	{
		for (auto& sessionManager : m_managers)
			sessionManager->Poll();
	}

	MatchClientSession* MatchSessions::CreateSession(std::shared_ptr<SessionBridge> bridge)
	{
		std::size_t sessionId = m_nextSessionId++;
		MatchClientSession* session = m_sessionPool.New<MatchClientSession>(m_match, sessionId, m_commandStore, std::move(bridge));

		m_sessionIdToSession.insert_or_assign(sessionId, session);

		bwLog(m_match.GetLogger(), LogLevel::Info, "Created session #{0}", sessionId);

		return session;
	}

	void MatchSessions::DeleteSession(MatchClientSession* session)
	{
		std::size_t sessionId = session->GetSessionId();
		m_sessionIdToSession.erase(sessionId);

		m_sessionPool.Delete(session);

		bwLog(m_match.GetLogger(), LogLevel::Info, "Deleted session #{0}", sessionId);
	}
}
