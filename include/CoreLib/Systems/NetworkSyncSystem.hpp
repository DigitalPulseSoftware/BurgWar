// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SYSTEMS_NETWORKSYNCSYSTEM_HPP
#define BURGWAR_CORELIB_SYSTEMS_NETWORKSYNCSYSTEM_HPP

#include <CoreLib/LayerIndex.hpp>
#include <CoreLib/Components/AnimationComponent.hpp>
#include <CoreLib/Components/HealthComponent.hpp>
#include <CoreLib/Components/InputComponent.hpp>
#include <CoreLib/Components/NetworkSyncComponent.hpp>
#include <CoreLib/Components/WeaponWielderComponent.hpp>
#include <CoreLib/Scripting/ScriptedElement.hpp>
#include <Nazara/Core/Signal.hpp>
#include <Nazara/Math/Angle.hpp>
#include <Nazara/Math/Vector2.hpp>
#include <NDK/System.hpp>
#include <Thirdparty/tsl/hopscotch_map.h>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace bw
{
	class TerrainLayer;

	class NetworkSyncSystem : public Ndk::System<NetworkSyncSystem>
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

			void NotifyPhysicsUpdate(const Ndk::EntityHandle& entity);
			void NotifyScaleUpdate(const Ndk::EntityHandle& entity);

			static Ndk::SystemIndex systemIndex;

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
				Ndk::EntityId entityId;
				std::size_t animId;
				Nz::UInt64 startTime;
			};

			struct EntityCreation
			{
				Ndk::EntityId entityId;
				EntityId uniqueId;
				Nz::RadianAnglef rotation;
				Nz::Vector2f position;
				float scale;
				std::string name;
				std::optional<Ndk::EntityId> parent;
				std::optional<Ndk::EntityId> weapon;
				std::optional<HealthProperties> healthProperties;
				std::optional<PlayerInputData> inputs;
				std::optional<PlayerMovementData> playerMovement;
				std::optional<PhysicsProperties> physicsProperties;
				std::string entityClass;
				tsl::hopscotch_map<std::string /*key*/, PropertyValue> properties;
				std::vector<std::pair<LayerIndex, Ndk::EntityId>> dependentIds;
			};

			struct EntityDeath
			{
				Ndk::EntityId entityId;
			};

			struct EntityDestruction
			{
				Ndk::EntityId entityId;
			};

			struct EntityHealth
			{
				Ndk::EntityId entityId;
				Nz::UInt16 currentHealth;
			};

			struct EntityInputs
			{
				Ndk::EntityId entityId;
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

				Ndk::EntityId entityId;
				bool isAsleep;
				float mass;
				float momentOfInertia;
				std::optional<PlayerMovement> playerMovement;
			};

			struct EntityScale
			{
				Ndk::EntityId entityId;
				float newScale;
			};

			struct EntityWeapon
			{
				Ndk::EntityId entityId;
				std::optional<Ndk::EntityId> weaponId;
			};

			struct EntityMovement
			{
				Ndk::EntityId entityId;
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
			void BuildEvent(EntityCreation& creationEvent, Ndk::Entity* entity) const;
			void BuildEvent(EntityDeath& deathEvent, Ndk::Entity* entity) const;
			void BuildEvent(EntityDestruction& deleteEvent, Ndk::Entity* entity) const;
			void BuildEvent(EntityMovement& movementEvent, Ndk::Entity* entity) const;

			void OnEntityAdded(Ndk::Entity* entity) override;
			void OnEntityRemoved(Ndk::Entity* entity) override;
			void OnUpdate(float elapsedTime) override;

			struct EntitySlots
			{
				NazaraSlot(AnimationComponent, OnAnimationStart, onAnimationStart);
				NazaraSlot(HealthComponent, OnDied, onDied);
				NazaraSlot(HealthComponent, OnHealthChange, onHealthChange);
				NazaraSlot(InputComponent, OnInputUpdate, onInputUpdate);
				NazaraSlot(NetworkSyncComponent, OnInvalidated, onInvalidated);
				NazaraSlot(WeaponWielderComponent, OnNewWeaponSelection, onNewWeaponSelection);
			};

			tsl::hopscotch_map<Ndk::EntityId, EntitySlots> m_entitySlots;

			Ndk::EntityList m_inputUpdateEntities;
			Ndk::EntityList m_invalidatedEntities;
			Ndk::EntityList m_healthUpdateEntities;
			Ndk::EntityList m_physicsEntities;
			Ndk::EntityList m_physicsUpdateEntities;
			Ndk::EntityList m_scaleUpdateEntities;
			Ndk::EntityList m_staticEntities;
			Ndk::EntityList m_weaponUpdateEntities;
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
