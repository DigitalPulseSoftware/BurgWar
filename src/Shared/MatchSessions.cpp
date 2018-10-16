// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/MatchSessions.hpp>
#include <Shared/MatchClientSession.hpp>
#include <iostream>

namespace bw
{
	MatchSessions::MatchSessions() :
	m_sessionPool(sizeof(MatchClientSession)),
	m_nextSessionId(0)
	{
	}

	MatchSessions::~MatchSessions()
	{
		//FIXME: This may call DeleteSession while iterating on it
		for (const auto& pair : m_sessionIdToSession)
			m_sessionPool.Delete(pair.second);
	}

	void MatchSessions::Poll()
	{
		for (auto& sessionManager : m_managers)
			sessionManager->Poll();
	}

	MatchClientSession* MatchSessions::CreateSession(std::unique_ptr<SessionBridge> bridge)
	{
		std::size_t sessionId = m_nextSessionId++;
		MatchClientSession* session = m_sessionPool.New<MatchClientSession>(sessionId, m_commandStore, std::move(bridge));

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
