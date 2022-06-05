// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SYSTEMS_NETWORKSYNCSYSTEM_HPP
#define BURGWAR_CORELIB_SYSTEMS_NETWORKSYNCSYSTEM_HPP

#include <CoreLib/Export.hpp>
#include <CoreLib/LayerIndex.hpp>
#include <CoreLib/Components/AnimationComponent.hpp>
#include <CoreLib/Components/HealthComponent.hpp>
#include <CoreLib/Components/InputComponent.hpp>
#include <CoreLib/Components/NetworkSyncComponent.hpp>
#include <CoreLib/Components/WeaponWielderComponent.hpp>
#include <CoreLib/Scripting/ScriptedElement.hpp>
#include <Nazara/Utils/Signal.hpp>
#include <Nazara/Math/Angle.hpp>
#include <Nazara/Math/Vector2.hpp>
#include <tsl/hopscotch_map.h>
#include <tsl/hopscotch_set.h>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace bw
{
	class Player;
	class TerrainLayer;

	class BURGWAR_CORELIB_API NetworkSyncSystem
	{
		public:
			struct EntityCreation;
			struct EntityDestruction;
			struct EntityMovement;

			NetworkSyncSystem(TerrainLayer& layer);
			~NetworkSyncSystem() = default;

			void CreateEntities(const std::function<void(const EntityCreation* entityCreation, std::size_t entityCount)>& callback) const;
			void DeleteEntities(const std::function<void(const EntityDestruction* entityDestruction, std::size_t entityCount)>& callback) const;
			
			inline TerrainLayer& GetLayer();
			inline const TerrainLayer& GetLayer() const;
			
			void MoveEntities(const std::function<void(const EntityMovement* entityMovement, std::size_t entityCount)>& callback) const;

			inline void NotifyPhysicsUpdate(entt::entity entity);
			inline void NotifyMovementUpdate(entt::entity entity);
			inline void NotifyScaleUpdate(entt::entity entity);

			struct HealthProperties
			{
				Nz::UInt16 currentHealth;
				Nz::UInt16 maxHealth;
			};

			struct PlayerMovementData
			{
				bool isFacingRight;
			};

			struct PhysicsProperties
			{
				Nz::RadianAnglef angularVelocity;
				Nz::Vector2f linearVelocity;
				bool isSleeping;
				float mass;
				float momentOfInertia;
			};

			struct EntityPlayAnimation
			{
				entt::entity entityId;
				std::size_t animId;
				Nz::UInt64 startTime;
			};

			struct EntityCreation
			{
				entt::entity entityId;
				EntityId uniqueId;
				Nz::RadianAnglef rotation;
				Nz::Vector2f position;
				float scale;
				Player* playerOwner;
				std::optional<entt::entity> parent;
				std::optional<entt::entity> weapon;
				std::optional<HealthProperties> healthProperties;
				std::optional<PlayerInputData> inputs;
				std::optional<PlayerMovementData> playerMovement;
				std::optional<PhysicsProperties> physicsProperties;
				std::string entityClass;
				tsl::hopscotch_map<std::string /*key*/, PropertyValue> properties;
				std::vector<std::pair<LayerIndex, entt::entity>> dependentIds;
			};

			struct EntityDeath
			{
				entt::entity entityId;
			};

			struct EntityDestruction
			{
				entt::entity entityId;
			};

			struct EntityHealth
			{
				entt::entity entityId;
				Nz::UInt16 currentHealth;
			};

			struct EntityInputs
			{
				entt::entity entityId;
				PlayerInputData inputs;
			};

			struct EntityPhysics
			{
				struct PlayerMovement
				{
					float jumpHeight;
					float jumpHeightBoost;
					float movementSpeed;
				};

				entt::entity entityId;
				bool isAsleep;
				float mass;
				float momentOfInertia;
				std::optional<PlayerMovement> playerMovement;
			};

			struct EntityScale
			{
				entt::entity entityId;
				float newScale;
			};

			struct EntityWeapon
			{
				entt::entity entityId;
				std::optional<entt::entity> weaponId;
			};

			struct EntityMovement
			{
				entt::entity entityId;
				Nz::RadianAnglef rotation;
				Nz::Vector2f position;
				std::optional<PlayerMovementData> playerMovement;
				std::optional<PhysicsProperties> physicsProperties;
			};

			NazaraSignal(OnEntityCreated, NetworkSyncSystem* /*emitter*/, const EntityCreation& /*event*/);
			NazaraSignal(OnEntityDeath, NetworkSyncSystem* /*emitter*/, const EntityDeath& /*event*/);
			NazaraSignal(OnEntityDeleted, NetworkSyncSystem* /*emitter*/, const EntityDestruction& /*event*/);
			NazaraSignal(OnEntityPlayAnimation, NetworkSyncSystem* /*emitter*/, const EntityPlayAnimation& /*event*/);
			NazaraSignal(OnEntityInvalidated, NetworkSyncSystem* /*emitter*/, const EntityMovement& /*event*/);
			NazaraSignal(OnEntitiesInputUpdate, NetworkSyncSystem* /*emitter*/, const EntityInputs* /*events*/, std::size_t /*entityCount*/);
			NazaraSignal(OnEntitiesHealthUpdate, NetworkSyncSystem* /*emitter*/, const EntityHealth* /*events*/, std::size_t /*entityCount*/);
			NazaraSignal(OnEntitiesPhysicsUpdate, NetworkSyncSystem* /*emitter*/, const EntityPhysics* /*events*/, std::size_t /*entityCount*/);
			NazaraSignal(OnEntitiesScaleUpdate, NetworkSyncSystem* /*emitter*/, const EntityScale* /*events*/, std::size_t /*entityCount*/);
			NazaraSignal(OnEntitiesWeaponUpdate, NetworkSyncSystem* /*emitter*/, const EntityWeapon* /*events*/, std::size_t /*entityCount*/);

		private:
			void BuildEvent(EntityCreation& creationEvent, entt::entity entity) const;
			void BuildEvent(EntityDeath& deathEvent, entt::entity entity) const;
			void BuildEvent(EntityDestruction& deleteEvent, entt::entity entity) const;
			void BuildEvent(EntityMovement& movementEvent, entt::entity entity) const;

			//void OnEntityAdded(entt::entity entity) override;
			//void OnEntityRemoved(entt::entity entity) override;
			//void OnUpdate(float elapsedTime) override;

			struct EntitySlots
			{
				NazaraSlot(AnimationComponent, OnAnimationStart, onAnimationStart);
				NazaraSlot(HealthComponent, OnDie, onDied);
				NazaraSlot(HealthComponent, OnHealthChange, onHealthChange);
				NazaraSlot(InputComponent, OnInputUpdate, onInputUpdate);
				NazaraSlot(NetworkSyncComponent, OnInvalidated, onInvalidated);
				NazaraSlot(WeaponWielderComponent, OnNewWeaponSelection, onNewWeaponSelection);
			};

			tsl::hopscotch_map<entt::entity, EntitySlots> m_entitySlots;

			tsl::hopscotch_set<entt::entity> m_inputUpdateEntities;
			tsl::hopscotch_set<entt::entity> m_healthUpdateEntities;
			tsl::hopscotch_set<entt::entity> m_movedStaticEntities;
			tsl::hopscotch_set<entt::entity> m_physicsEntities;
			tsl::hopscotch_set<entt::entity> m_physicsUpdateEntities;
			tsl::hopscotch_set<entt::entity> m_scaleUpdateEntities;
			tsl::hopscotch_set<entt::entity> m_staticEntities;
			tsl::hopscotch_set<entt::entity> m_weaponUpdateEntities;
			mutable std::vector<EntityCreation> m_creationEvents;
			mutable std::vector<EntityDestruction> m_destructionEvents;
			std::vector<EntityHealth> m_healthEvents;
			std::vector<EntityInputs> m_inputEvents;
			std::vector<EntityPhysics> m_physicsEvent;
			std::vector<EntityScale> m_scaleEvent;
			std::vector<EntityWeapon> m_weaponEvents;
			mutable std::vector<EntityMovement> m_movementEvents;
			TerrainLayer& m_layer;
	};
}

#include <CoreLib/Systems/NetworkSyncSystem.inl>

#endif
