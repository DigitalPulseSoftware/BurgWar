// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/SessionManager.hpp>

namespace bw
{
	inline SessionManager::SessionManager(MatchSessions* owner) :
	m_owner(owner)
	{
	}

	inline MatchSessions* SessionManager::GetOwner()
	{
		return m_owner;
	}
}
