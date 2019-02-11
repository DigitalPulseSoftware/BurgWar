// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <GameLibShared/SessionBridge.hpp>

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