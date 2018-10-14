// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/SessionBridge.hpp>

namespace bw
{
	inline SessionBridge::SessionBridge(MatchClientSession* session) :
	m_session(session)
	{
	}

	inline MatchClientSession* SessionBridge::GetSession()
	{
		return m_session;
	}
}