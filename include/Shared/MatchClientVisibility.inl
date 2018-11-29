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

	template<typename T>
	void MatchClientVisibility::SendEntityPacket(Nz::UInt32 entityId, T&& packet)
	{
		auto it = m_pendingEntitiesEvent.find(entityId);
		if (it == m_pendingEntitiesEvent.end())
			it = m_pendingEntitiesEvent.emplace(entityId, std::vector<EntityPacketSendFunction>()).first;

		it.value().emplace_back([this, packet = std::forward<T>(packet)]() { m_session.SendPacket(packet); });
	}
}