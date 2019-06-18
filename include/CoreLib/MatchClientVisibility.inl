// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/MatchClientVisibility.hpp>

namespace bw
{
	namespace Detail
	{
		template<typename T> 
		struct HasStateTick 
		{
			using UnrefT = std::decay_t<T>;
			struct Fallback { Nz::UInt16 stateTick; };
			struct Derived : UnrefT, Fallback { };

			template<typename C, C> struct ChT;

			template<typename C> static char(&f(ChT<Nz::UInt16 Fallback::*, &C::stateTick>*))[1];
			template<typename C> static char(&f(...))[2];

			static bool const value = sizeof(f<Derived>(0)) == 2;
		};
	}

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
	void MatchClientVisibility::PushEntityPacket(Nz::UInt32 entityId, T&& packet)
	{
		auto it = m_pendingEntitiesEvent.find(entityId);
		if (it == m_pendingEntitiesEvent.end())
			it = m_pendingEntitiesEvent.emplace(entityId, std::vector<EntityPacketSendFunction>()).first;

		if constexpr (Detail::HasStateTick<T>::value)
		{
			it.value().emplace_back([this, packet = std::forward<T>(packet)]() mutable
			{
				packet.stateTick = m_match.GetCurrentTick();

				m_session.SendPacket(packet);
			});
		}
		else
		{
			it.value().emplace_back([this, packet = std::forward<T>(packet)]() mutable
			{
				m_session.SendPacket(packet);
			});
		}
	}

	template<typename T>
	void MatchClientVisibility::PushEntitiesPacket(Nz::Bitset<Nz::UInt64> entitiesId, T&& packet)
	{
		if constexpr (Detail::HasStateTick<T>::value)
		{
			m_multiplePendingEntitiesEvent.emplace_back(PendingMultipleEntities{
				std::move(entitiesId),
				[this, packet = std::forward<T>(packet)]() mutable
				{
					packet.stateTick = m_match.GetCurrentTick();

					m_session.SendPacket(packet);
				}
			});
		}
		else
		{
			m_multiplePendingEntitiesEvent.emplace_back(PendingMultipleEntities{
				std::move(entitiesId),
				[this, packet = std::forward<T>(packet)]() mutable
				{
					m_session.SendPacket(packet);
				}
			});
		}
	}
}