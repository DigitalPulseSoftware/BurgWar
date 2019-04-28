// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/MatchClientVisibility.hpp>

namespace bw
{
	inline MatchClientVisibility::MatchClientVisibility(Match& match, MatchClientSession& session) :
	m_match(match),
	m_session(session),
	m_activeLayer(NoLayer)
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