// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/MatchSessions.hpp>

namespace bw
{
	template<typename T, typename ...Args>
	T* MatchSessions::CreateSessionManager(Args&&... args)
	{
		m_managers.emplace_back(std::make_unique<T>(this, std::forward<Args>(args)...));
		return static_cast<T*>(m_managers.back().get());
	}

	template<typename F>
	void MatchSessions::ForEachSession(F&& cb)
	{
		for (MatchClientSession& session : m_sessionPool)
			cb(session);
	}

	inline Match& MatchSessions::GetMatch()
	{
		return m_match;
	}
}
