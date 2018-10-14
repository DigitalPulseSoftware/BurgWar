// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/SessionManager.hpp>

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