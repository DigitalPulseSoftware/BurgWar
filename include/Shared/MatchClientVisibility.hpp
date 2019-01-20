// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_SERVER_CLIENTVISIBILITY_HPP
#define BURGWAR_SERVER_CLIENTVISIBILITY_HPP

#include <Nazara/Core/Bitset.hpp>
#include <Nazara/Core/Signal.hpp>
#include <NDK/EntityList.hpp>
#include <Shared/MatchClientSession.hpp>
#include <Shared/Protocol/Packets.hpp>
#include <Shared/Components/HealthComponent.hpp>
#include <Shared/Systems/NetworkSyncSystem.hpp>
#include <hopscotch/hopscotch_map.h>
#include <hopscotch/hopscotch_set.h>
#include <limits>
#include <vector>

namespace bw
{
	class Match;
	class MatchClientSession;

	class MatchClientVisibility final
	{
		public:
			inline MatchClientVisibility(Match& match, MatchClientSession& session);
			~MatchClientVisibility() = default;

			inline std::size_t GetActiveLayer() const;

			template<typename T> void SendEntityPacket(Nz::UInt32 entityId, T&& packet);

			void Update(float elapsedTime);
			void UpdateLayer(std::size_t layerIndex);

			static constexpr std::size_t NoLayer = std::numeric_limits<std::size_t>::max();

		private:
			void HandleEntityCreation(const NetworkSyncSystem::EntityCreation& eventData);
			void HandleEntityDestruction(const NetworkSyncSystem::EntityDestruction& eventData);
			void SendMatchState(float elapsedTime);

			NazaraSlot(NetworkSyncSystem, OnEntityCreated, m_onEntityCreatedSlot);
			NazaraSlot(NetworkSyncSystem, OnEntityDeleted, m_onEntityDeletedSlot);
			NazaraSlot(NetworkSyncSystem, OnEntityPlayAnimation, m_onEntityPlayAnimation);
			NazaraSlot(NetworkSyncSystem, OnEntitiesHealthUpdate, m_onEntitiesHealthUpdate);
			NazaraSlot(NetworkSyncSystem, OnEntitiesInputUpdate, m_onEntitiesInputUpdate);

			using EntityPacketSendFunction = std::function<void()>;
			using PendingCreationEventMap = tsl::hopscotch_map<Nz::UInt32 /*entityId*/, std::optional<NetworkSyncSystem::EntityCreation>>;

			std::size_t m_activeLayer;
			PendingCreationEventMap m_creationEvents;
			tsl::hopscotch_map<Nz::UInt32 /*entityId*/, NetworkSyncSystem::EntityInputs> m_inputUpdateEvents;
			tsl::hopscotch_map<Nz::UInt32 /*entityId*/, NetworkSyncSystem::EntityHealth> m_healthUpdateEvents;
			tsl::hopscotch_map<Nz::UInt32 /*entityId*/, NetworkSyncSystem::EntityPlayAnimation> m_playAnimationEvents;
			tsl::hopscotch_set<Nz::UInt32 /*entityId*/> m_destructionEvents;
			tsl::hopscotch_map<Nz::UInt32 /*entityId*/, std::vector<EntityPacketSendFunction>> m_pendingEntitiesEvent;
			Nz::Bitset<Nz::UInt64> m_visibleEntities;
			Match& m_match;
			MatchClientSession& m_session;
			float m_entityMovementSendInterval;
			float m_entityMovementSendTimer;

			Packets::CreateEntities m_createEntitiesPacket;
			Packets::DeleteEntities m_deleteEntitiesPacket;
			Packets::HealthUpdate   m_healthUpdatePacket;
			Packets::EntitiesInputs m_inputUpdatePacket;
			Packets::MatchState     m_matchStatePacket;
	};
}

#include <Shared/MatchClientVisibility.inl>

#endif