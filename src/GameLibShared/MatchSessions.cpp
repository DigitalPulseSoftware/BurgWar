// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <GameLibShared/MatchSessions.hpp>
#include <GameLibShared/MatchClientSession.hpp>
#include <iostream>

namespace bw
{
	MatchSessions::MatchSessions(Match& match) :
	m_match(match),
	m_sessionPool(sizeof(MatchClientSession)),
	m_nextSessionId(0)
	{
	}

	MatchSessions::~MatchSessions()
	{
		for (const auto& pair : m_sessionIdToSession)
			m_sessionPool.Delete(pair.second);
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

		std::cout << "Created session #" << sessionId << std::endl;

		return session;
	}

	void MatchSessions::DeleteSession(MatchClientSession* session)
	{
		std::size_t sessionId = session->GetSessionId();
		m_sessionIdToSession.erase(sessionId);

		m_sessionPool.Delete(session);

		std::cout << "Deleted session #" << sessionId << std::endl;
	}
}
