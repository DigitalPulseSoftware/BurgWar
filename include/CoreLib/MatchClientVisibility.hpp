// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_SERVER_CLIENTVISIBILITY_HPP
#define BURGWAR_SERVER_CLIENTVISIBILITY_HPP

#include <Nazara/Core/Bitset.hpp>
#include <Nazara/Core/Signal.hpp>
#include <NDK/EntityList.hpp>
#include <CoreLib/Match.hpp>
#include <CoreLib/MatchClientSession.hpp>
#include <CoreLib/Protocol/Packets.hpp>
#include <CoreLib/Components/HealthComponent.hpp>
#include <CoreLib/Systems/NetworkSyncSystem.hpp>
#include <tsl/hopscotch_map.h>
#include <tsl/hopscotch_set.h>
#include <limits>
#include <vector>

namespace bw
{
	class MatchClientSession;

	class MatchClientVisibility final
	{
		public:
			inline MatchClientVisibility(Match& match, MatchClientSession& session);
			~MatchClientVisibility() = default;

			void HideLayer(std::size_t layerIndex);

			inline bool IsLayerVisible(std::size_t layerIndex) const;

			template<typename T> void PushEntityPacket(Nz::UInt16 layerIndex, Nz::UInt32 entityId, T&& packet);
			template<typename T> void PushEntitiesPacket(Nz::UInt16 layerIndex, Nz::Bitset<Nz::UInt64> entitiesId, T&& packet);

			void ShowLayer(std::size_t layerIndex);

			void Update();

			static inline Nz::UInt64 BuildEntityId(Nz::UInt16 layerIndex, Nz::UInt32 entityId);
			static inline Packets::Helper::EntityId DecodeEntityId(Nz::UInt64 entityId);

		private:
			void HandleEntityCreation(Nz::UInt16 layerIndex, const NetworkSyncSystem::EntityCreation& eventData);
			void HandleEntityDestruction(Nz::UInt16 layerIndex, const NetworkSyncSystem::EntityDestruction& eventData, bool clearDeath);
			void SendMatchState();

			void BuildMovementPacket(Nz::UInt16 layerIndex, Packets::MatchState::Entity& packetData, const NetworkSyncSystem::EntityMovement& eventData);

			using EntityPacketSendFunction = std::function<void()>;
			using PendingCreationEventMap = tsl::hopscotch_map<Nz::UInt64 /*entityId*/, std::optional<NetworkSyncSystem::EntityCreation>>;

			struct PendingMultipleEntities
			{
				Nz::UInt16 layerIndex;
				Nz::Bitset<Nz::UInt64> entitiesId;
				EntityPacketSendFunction sendFunction;
			};

			struct Layer
			{
				Nz::Bitset<Nz::UInt64> visibleEntities;

				NazaraSlot(NetworkSyncSystem, OnEntityCreated,        onEntityCreatedSlot);
				NazaraSlot(NetworkSyncSystem, OnEntityDeleted,        onEntityDeletedSlot);
				NazaraSlot(NetworkSyncSystem, OnEntityInvalidated,    onEntityInvalidated);
				NazaraSlot(NetworkSyncSystem, OnEntityPlayAnimation,  onEntityPlayAnimation);
				NazaraSlot(NetworkSyncSystem, OnEntitiesDeath,        onEntitiesDeath);
				NazaraSlot(NetworkSyncSystem, OnEntitiesHealthUpdate, onEntitiesHealthUpdate);
				NazaraSlot(NetworkSyncSystem, OnEntitiesInputUpdate,  onEntitiesInputUpdate);
			};

			Nz::Bitset<Nz::UInt64> m_tempBitset; //< For optimization purpose
			Nz::Bitset<Nz::UInt64> m_visibleLayers;
			tsl::hopscotch_map<Nz::UInt16 /*layerId*/, Layer> m_layers;
			tsl::hopscotch_map<Nz::UInt64 /*entityId*/, NetworkSyncSystem::EntityInputs> m_inputUpdateEvents;
			tsl::hopscotch_map<Nz::UInt64 /*entityId*/, NetworkSyncSystem::EntityHealth> m_healthUpdateEvents;
			tsl::hopscotch_map<Nz::UInt64 /*entityId*/, NetworkSyncSystem::EntityMovement> m_staticMovementUpdateEvents;
			tsl::hopscotch_map<Nz::UInt64 /*entityId*/, NetworkSyncSystem::EntityPlayAnimation> m_playAnimationEvents;
			tsl::hopscotch_set<Nz::UInt64 /*entityId*/> m_deathEvents;
			tsl::hopscotch_set<Nz::UInt64 /*entityId*/> m_destructionEvents;
			tsl::hopscotch_map<Nz::UInt64 /*entityId*/, std::vector<EntityPacketSendFunction>> m_pendingEntitiesEvent;
			std::vector<PendingMultipleEntities> m_multiplePendingEntitiesEvent;
			PendingCreationEventMap m_creationEvents;
			Match& m_match;
			MatchClientSession& m_session;

			Packets::CreateEntities    m_createEntitiesPacket;
			Packets::DeleteEntities    m_deleteEntitiesPacket;
			Packets::HealthUpdate      m_healthUpdatePacket;
			Packets::EntitiesAnimation m_entitiesAnimationPacket;
			Packets::EntitiesDeath     m_entitiesDeathPacket;
			Packets::EntitiesInputs    m_inputUpdatePacket;
			Packets::MatchState        m_matchStatePacket;
	};
}

#include <CoreLib/MatchClientVisibility.inl>

#endif