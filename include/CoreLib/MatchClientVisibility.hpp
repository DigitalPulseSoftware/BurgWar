// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_SERVER_CLIENTVISIBILITY_HPP
#define BURGWAR_SERVER_CLIENTVISIBILITY_HPP

#include <Nazara/Core/Bitset.hpp>
#include <Nazara/Core/Flags.hpp>
#include <Nazara/Core/Signal.hpp>
#include <NDK/EntityList.hpp>
#include <CoreLib/LayerIndex.hpp>
#include <CoreLib/Match.hpp>
#include <CoreLib/MatchClientSession.hpp>
#include <CoreLib/Protocol/Packets.hpp>
#include <CoreLib/Components/HealthComponent.hpp>
#include <CoreLib/Systems/NetworkSyncSystem.hpp>
#include <Thirdparty/tsl/hopscotch_map.h>
#include <Thirdparty/tsl/hopscotch_set.h>
#include <limits>
#include <vector>

namespace bw
{
	enum class VisibilityEventType
	{
		Creation,
		Death,
		Destruction,
		HealthUpdate,
		InputUpdate,
		PlayAnimation,
		PhysicsUpdate,
		WeaponUpdate,

		Max = WeaponUpdate
	};
}

namespace Nz
{
	template<>
	struct EnumAsFlags<bw::VisibilityEventType>
	{
		static constexpr bw::VisibilityEventType max = bw::VisibilityEventType::Max;
	};
}

namespace bw
{
	class MatchClientSession;

	class MatchClientVisibility final
	{
		public:
			inline MatchClientVisibility(Match& match, MatchClientSession& session);
			~MatchClientVisibility() = default;

			inline void ClearLayers();

			inline void HideLayer(LayerIndex layerIndex);

			inline bool IsLayerVisible(LayerIndex layerIndex) const;

			template<typename T> void PushEntityPacket(LayerIndex layerIndex, Nz::UInt32 entityId, T&& packet);
			template<typename T> void PushEntitiesPacket(LayerIndex layerIndex, Nz::Bitset<Nz::UInt64> entitiesId, T&& packet);
			inline void PushLayerUpdate(Nz::UInt8 localPlayerIndex, LayerIndex layerIndex);

			inline void SetEntityControlledStatus(LayerIndex layerIndex, Nz::UInt32 entityId, bool isControlled);

			void ShowLayer(LayerIndex layerIndex);

			void Update();

		private:
			struct PriorityMovementData
			{
				Nz::UInt8 priorityAccumulator;
				LayerIndex layerIndex;
				NetworkSyncSystem::EntityMovement movementData;
				bool staticEntity;
			};

			void BuildMovementPacket(Packets::MatchState::Entity& packetData, const NetworkSyncSystem::EntityMovement& eventData);
			void FillEntityData(const NetworkSyncSystem::EntityCreation& creationEvent, Packets::Helper::EntityData& entityData);
			void HandleEntityCreation(LayerIndex layerIndex, const NetworkSyncSystem::EntityCreation& eventData);
			void HandleEntityRemove(LayerIndex layerIndex, Ndk::EntityId entityId, bool deathEvent);
			void SendMatchState();

			using EntityPacketSendFunction = std::function<void()>;
			using PendingCreationEventMap = tsl::hopscotch_map<Nz::UInt32 /*entityId*/, std::optional<NetworkSyncSystem::EntityCreation>>;

			struct PendingLayerUpdate
			{
				Nz::UInt8 localPlayerIndex;
				LayerIndex layerIndex;
			};

			struct PendingMultipleEntities
			{
				LayerIndex layerIndex;
				Nz::Bitset<Nz::UInt64> entitiesId;
				EntityPacketSendFunction sendFunction;
			};

			struct Layer
			{
				struct VisibleEntityData
				{
					Nz::UInt8 priorityAccumulator = 0;
				};

				std::size_t visibilityCounter = 1;

				PendingCreationEventMap creationEvents;
				tsl::hopscotch_map<Nz::UInt32 /*entityId*/, NetworkSyncSystem::EntityInputs> inputUpdateEvents;
				tsl::hopscotch_map<Nz::UInt32 /*entityId*/, NetworkSyncSystem::EntityHealth> healthUpdateEvents;
				tsl::hopscotch_map<Nz::UInt32 /*entityId*/, NetworkSyncSystem::EntityMovement> staticMovementUpdateEvents;
				tsl::hopscotch_map<Nz::UInt32 /*entityId*/, NetworkSyncSystem::EntityPlayAnimation> playAnimationEvents;
				tsl::hopscotch_map<Nz::UInt32 /*entityId*/, NetworkSyncSystem::EntityPhysics> physicsEvents;
				tsl::hopscotch_map<Nz::UInt32 /*entityId*/, NetworkSyncSystem::EntityWeapon> weaponEvents;
				tsl::hopscotch_map<Nz::UInt32 /*entityId*/, VisibleEntityData> visibleEntities;
				tsl::hopscotch_set<Nz::UInt32 /*entityId*/> deathEvents;
				tsl::hopscotch_set<Nz::UInt32 /*entityId*/> destructionEvents;

				NazaraSlot(NetworkSyncSystem, OnEntityCreated,         onEntityCreatedSlot);
				NazaraSlot(NetworkSyncSystem, OnEntityDeath,           onEntityDeath);
				NazaraSlot(NetworkSyncSystem, OnEntityDeleted,         onEntityDeletedSlot);
				NazaraSlot(NetworkSyncSystem, OnEntityInvalidated,     onEntityInvalidated);
				NazaraSlot(NetworkSyncSystem, OnEntityPlayAnimation,   onEntityPlayAnimation);
				NazaraSlot(NetworkSyncSystem, OnEntitiesHealthUpdate,  onEntitiesHealthUpdate);
				NazaraSlot(NetworkSyncSystem, OnEntitiesInputUpdate,   onEntitiesInputUpdate);
				NazaraSlot(NetworkSyncSystem, OnEntitiesPhysicsUpdate, onEntitiesPhysicsUpdate);
				NazaraSlot(NetworkSyncSystem, OnEntitiesWeaponUpdate,  onEntitiesWeaponUpdate);
			};

			Nz::Bitset<Nz::UInt64> m_newlyHiddenLayers;
			Nz::Bitset<Nz::UInt64> m_newlyVisibleLayers;
			Nz::Bitset<Nz::UInt64> m_clientVisibleLayers;
			Nz::Flags<VisibilityEventType> m_pendingEvents;
			tsl::hopscotch_map<LayerIndex /*layerId*/, std::unique_ptr<Layer>> m_layers;
			tsl::hopscotch_map<Nz::UInt64 /*layerId|entityId*/, std::vector<EntityPacketSendFunction>> m_pendingEntitiesEvent;
			tsl::hopscotch_set<Nz::UInt64 /*layerId|entityId*/> m_controlledEntities;
			std::vector<PendingLayerUpdate> m_pendingLayerUpdates;
			std::vector<PendingMultipleEntities> m_multiplePendingEntitiesEvent;
			std::vector<PriorityMovementData> m_priorityMovementData;
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
