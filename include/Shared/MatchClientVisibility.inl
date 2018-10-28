// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/MatchClientVisibility.hpp>

namespace bw
{
	inline MatchClientVisibility::MatchClientVisibility(Match& match, MatchClientSession& session) :
	m_match(match),
	m_session(session),
	m_activeLayer(NoLayer),
	m_entityMovementSendInterval(1.f / 20.f),
	m_entityMovementSendTimer(0.f)
	{
	}
	
	inline std::size_t MatchClientVisibility::GetActiveLayer() const
	{
		return m_activeLayer;
	}
}