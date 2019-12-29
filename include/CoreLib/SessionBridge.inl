// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/SessionBridge.hpp>
#include <cassert>

namespace bw
{
	inline SessionBridge::SessionBridge(MatchClientSession* session) :
	m_session(session),
	m_isConnected(false)
	{
	}

	inline MatchClientSession* SessionBridge::GetSession()
	{
		return m_session;
	}

	inline bool SessionBridge::IsConnected() const
	{
		return m_isConnected;
	}
}