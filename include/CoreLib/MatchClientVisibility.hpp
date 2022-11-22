// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_SERVER_CLIENTVISIBILITY_HPP
#define BURGWAR_SERVER_CLIENTVISIBILITY_HPP

#include <CoreLib/Export.hpp>
#include <CoreLib/LayerIndex.hpp>
#include <CoreLib/Match.hpp>
#include <CoreLib/MatchClientSession.hpp>
#include <CoreLib/Protocol/Packets.hpp>
#include <CoreLib/Components/HealthComponent.hpp>
#include <CoreLib/Systems/NetworkSyncSystem.hpp>
#include <Nazara/Utils/Bitset.hpp>
#include <Nazara/Utils/Flags.hpp>
#include <Nazara/Utils/Signal.hpp>
#include <tsl/hopscotch_map.h>
#include <tsl/hopscotch_set.h>
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
		ScaleUpdate,
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

	class BURGWAR_CORELIB_API MatchClientVisibility final
	{
		public:
			inline MatchClientVisibility(Match& match, MatchClientSession& session);
			MatchClientVisibility(const MatchClientVisibility&) = delete;
			MatchClientVisibility(MatchClientVisibility&&) noexcept = default;
			~MatchClientVisibility() = default;

			inline void ClearLayers();

			inline void HideLayer(LayerIndex layerIndex);

			inline bool IsLayerVisible(LayerIndex layerIndex) const;

			template<typename T> void PushEntityPacket(LayerIndex layerIndex, Nz::UInt32 entityId, T&& packet);
			template<typename T> void PushEntitiesPacket(LayerIndex layerIndex, Nz::Bitset<Nz::UInt64> entitiesId, T&& packet);
			inline void PushLayerUpdate(Nz::UInt8 localPlayerIndex, LayerIndex layerIndex);

			void ResetVisibleEntities();

			inline void SetEntityControlledStatus(LayerIndex layerIndex, Nz::UInt32 entityId, bool isControlled);
			inline void ShouldIgnoreEvents(bool ignoreEvents);

			void ShowLayer(LayerIndex layerIndex);

			void Update();

			MatchClientVisibility& operator=(const MatchClientVisibility&) = delete;
			MatchClientVisibility& operator=(MatchClientVisibility&&) = delete;

		private:
			struct PriorityMovementData
			{
				Nz::UInt8 priorityAccumulator;
				LayerIndex layerIndex;
				NetworkSyncSystem::EntityMovement movementData;
				bool staticEntity;
			};

			using EntityPacketSendFunction = std::function<void()>;
			using PendingCreationEventMap = tsl::hopscotch_map<Nz::UInt32 /*entityId*/, std::optional<NetworkSyncSystem::EntityCreation>>;

			void BuildMovementPacket(Packets::MatchState::Entity& packetData, const NetworkSyncSystem::EntityMovement& eventData);
			void FillEntityData(const NetworkSyncSystem::EntityCreation& creationEvent, Packets::Helper::EntityData& entityData);
			void HandleEntityCreation(LayerIndex layerIndex, const NetworkSyncSystem::EntityCreation& eventData);
			void HandleEntityRemove(LayerIndex layerIndex, Nz::UInt32 networkId, bool deathEvent);
			template<typename E> void PushLayerEntities(std::vector<E>& packetEntities, LayerIndex layerIndex, PendingCreationEventMap& pendingCreationMap);
			void SendMatchState();

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
				tsl::hopscotch_map<Nz::UInt32 /*networkId*/, NetworkSyncSystem::EntityInputs> inputUpdateEvents;
				tsl::hopscotch_map<Nz::UInt32 /*networkId*/, NetworkSyncSystem::EntityHealth> healthUpdateEvents;
				tsl::hopscotch_map<Nz::UInt32 /*networkId*/, NetworkSyncSystem::EntityMovement> staticMovementUpdateEvents;
				tsl::hopscotch_map<Nz::UInt32 /*networkId*/, NetworkSyncSystem::EntityPlayAnimation> playAnimationEvents;
				tsl::hopscotch_map<Nz::UInt32 /*networkId*/, NetworkSyncSystem::EntityPhysics> physicsEvents;
				tsl::hopscotch_map<Nz::UInt32 /*networkId*/, NetworkSyncSystem::EntityScale> scaleEvents;
				tsl::hopscotch_map<Nz::UInt32 /*networkId*/, NetworkSyncSystem::EntityWeapon> weaponEvents;
				tsl::hopscotch_map<Nz::UInt32 /*networkId*/, VisibleEntityData> visibleEntities;
				tsl::hopscotch_set<Nz::UInt32 /*networkId*/> deathEvents;
				tsl::hopscotch_set<Nz::UInt32 /*networkId*/> destructionEvents;

				NazaraSlot(NetworkSyncSystem, OnEntityCreated,         onEntityCreatedSlot);
				NazaraSlot(NetworkSyncSystem, OnEntityDeath,           onEntityDeath);
				NazaraSlot(NetworkSyncSystem, OnEntityDeleted,         onEntityDeletedSlot);
				NazaraSlot(NetworkSyncSystem, OnEntityInvalidated,     onEntityInvalidated);
				NazaraSlot(NetworkSyncSystem, OnEntityPlayAnimation,   onEntityPlayAnimation);
				NazaraSlot(NetworkSyncSystem, OnEntitiesHealthUpdate,  onEntitiesHealthUpdate);
				NazaraSlot(NetworkSyncSystem, OnEntitiesInputUpdate,   onEntitiesInputUpdate);
				NazaraSlot(NetworkSyncSystem, OnEntitiesPhysicsUpdate, onEntitiesPhysicsUpdate);
				NazaraSlot(NetworkSyncSystem, OnEntitiesScaleUpdate,   onEntitiesScaleUpdate);
				NazaraSlot(NetworkSyncSystem, OnEntitiesWeaponUpdate,  onEntitiesWeaponUpdate);
			};

			Nz::Bitset<Nz::UInt64> m_newlyHiddenLayers;
			Nz::Bitset<Nz::UInt64> m_newlyVisibleLayers;
			Nz::Bitset<Nz::UInt64> m_clientVisibleLayers;
			Nz::Flags<VisibilityEventType> m_pendingEvents;
			tsl::hopscotch_map<LayerIndex /*layerId*/, std::unique_ptr<Layer>> m_layers;
			tsl::hopscotch_map<Nz::UInt64 /*layerId|networkId*/, std::vector<EntityPacketSendFunction>> m_pendingEntitiesEvent;
			tsl::hopscotch_set<Nz::UInt64 /*layerId|networkId*/> m_controlledEntities;
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
			Packets::EntitiesScale     m_scaleUpdatePacket;
			Packets::MatchState        m_matchStatePacket;
			bool m_ignoreEvents;
	};
}

#include <CoreLib/MatchClientVisibility.inl>

#endif
