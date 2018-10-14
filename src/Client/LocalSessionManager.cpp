// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/LocalSessionManager.hpp>
#include <Shared/MatchSessions.hpp>
#include <iostream>

namespace bw
{
	LocalSessionManager::LocalSessionManager(MatchSessions* owner) :
	SessionManager(owner)
	{
	}

	LocalSessionManager::~LocalSessionManager() = default;

	void LocalSessionManager::Poll()
	{
	}
}
