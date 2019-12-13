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
	m_session(session)
	{
	}

	inline void MatchClientVisibility::ClearLayers()
	{
		for (auto&& [layerIndex, layer] : m_layers)
		{
			m_newlyVisibleLayers.UnboundedReset(layerIndex);

			if (m_clientVisibleLayers.UnboundedTest(layerIndex))
				m_newlyHiddenLayers.UnboundedSet(layerIndex);
		}

		m_layers.clear();
	}

	inline void MatchClientVisibility::HideLayer(LayerIndex layerIndex)
	{
		auto it = m_layers.find(layerIndex);
		assert(it != m_layers.end());
		auto& layer = *it.value();
		if (--layer.visibilityCounter > 0)
			return;

		m_newlyVisibleLayers.UnboundedReset(layerIndex);

		if (m_clientVisibleLayers.UnboundedTest(layerIndex))
			m_newlyHiddenLayers.UnboundedSet(layerIndex);

		m_layers.erase(it);
	}

	inline bool MatchClientVisibility::IsLayerVisible(LayerIndex layerIndex) const
	{
		return m_layers.find(layerIndex) != m_layers.end();
	}

	inline void MatchClientVisibility::PushLayerUpdate(Nz::UInt8 playerIndex, LayerIndex layerIndex)
	{
		m_pendingLayerUpdates.emplace_back(PendingLayerUpdate{ playerIndex, layerIndex });
	}

	inline void MatchClientVisibility::SetEntityControlledStatus(LayerIndex layerIndex, Nz::UInt32 entityId, bool isControlled)
	{
		Nz::UInt64 entityKey = Nz::UInt64(layerIndex) << 32 | entityId;
		if (isControlled)
			m_controlledEntities.insert(entityKey);
		else
			m_controlledEntities.erase(entityKey);
	}

	template<typename T>
	void MatchClientVisibility::PushEntityPacket(LayerIndex layerIndex, Nz::UInt32 entityId, T&& packet)
	{
		Nz::UInt64 entityKey = Nz::UInt64(layerIndex) << 32 | entityId;

		auto it = m_pendingEntitiesEvent.find(entityKey);
		if (it == m_pendingEntitiesEvent.end())
			it = m_pendingEntitiesEvent.emplace(entityKey, std::vector<EntityPacketSendFunction>()).first;

		if constexpr (Detail::HasStateTick<T>::value)
		{
			it.value().emplace_back([this, packet = std::forward<T>(packet)]() mutable
			{
				packet.stateTick = m_match.GetNetworkTick();

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
	void MatchClientVisibility::PushEntitiesPacket(LayerIndex layerIndex, Nz::Bitset<Nz::UInt64> entitiesId, T&& packet)
	{
		if constexpr (Detail::HasStateTick<T>::value)
		{
			m_multiplePendingEntitiesEvent.emplace_back(PendingMultipleEntities{
				layerIndex,
				std::move(entitiesId),
				[this, packet = std::forward<T>(packet)]() mutable
				{
					packet.stateTick = m_match.GetNetworkTick();

					m_session.SendPacket(packet);
				}
			});
		}
		else
		{
			m_multiplePendingEntitiesEvent.emplace_back(PendingMultipleEntities{
				layerIndex,
				std::move(entitiesId),
				[this, packet = std::forward<T>(packet)]() mutable
				{
					m_session.SendPacket(packet);
				}
			});
		}
	}
}