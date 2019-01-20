// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_SHARED_SYSTEMS_NETWORKSYNCSYSTEM_HPP
#define BURGWAR_SHARED_SYSTEMS_NETWORKSYNCSYSTEM_HPP

#include <Shared/Components/AnimationComponent.hpp>
#include <Shared/Components/HealthComponent.hpp>
#include <Shared/Components/InputComponent.hpp>
#include <Nazara/Core/Signal.hpp>
#include <Nazara/Math/Angle.hpp>
#include <Nazara/Math/Vector2.hpp>
#include <NDK/System.hpp>
#include <hopscotch/hopscotch_map.h>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace bw
{
	class NetworkSyncSystem : public Ndk::System<NetworkSyncSystem>
	{
		public:
			struct EntityCreation;
			struct EntityDestruction;
			struct EntityMovement;

			NetworkSyncSystem();
			~NetworkSyncSystem() = default;

			void CreateEntities(const std::function<void(const EntityCreation* entityCreation, std::size_t entityCount)>& callback) const;
			void DeleteEntities(const std::function<void(const EntityDestruction* entityDestruction, std::size_t entityCount)>& callback) const;
			void MoveEntities(const std::function<void(const EntityMovement* entityMovement, std::size_t entityCount)>& callback) const;

			static Ndk::SystemIndex systemIndex;

			struct HealthProperties
			{
				Nz::UInt16 currentHealth;
				Nz::UInt16 maxHealth;
			};

			struct PlayerMovementData
			{
				bool isAirControlling;
				bool isFacingRight;
			};

			struct PhysicsProperties
			{
				Nz::RadianAnglef angularVelocity;
				Nz::Vector2f linearVelocity;
			};

			struct EntityPlayAnimation
			{
				Ndk::EntityId entityId;
				std::size_t animId;
				Nz::UInt64 startTime;
			};

			struct EntityCreation
			{
				Ndk::EntityId id;
				Nz::RadianAnglef rotation;
				Nz::Vector2f position;
				std::optional<Ndk::EntityId> parent;
				std::optional<HealthProperties> healthProperties;
				std::optional<InputData> inputs;
				std::optional<PlayerMovementData> playerMovement;
				std::optional<PhysicsProperties> physicsProperties;
				std::string entityClass;
			};

			struct EntityDestruction
			{
				Ndk::EntityId id;
			};

			struct EntityHealth
			{
				Ndk::EntityId id;
				Nz::UInt16 currentHealth;
			};

			struct EntityInputs
			{
				Ndk::EntityId id;
				InputData inputs;
			};

			struct EntityMovement
			{
				Ndk::EntityId id;
				Nz::RadianAnglef rotation;
				Nz::Vector2f position;
				std::optional<PlayerMovementData> playerMovement;
				std::optional<PhysicsProperties> physicsProperties;
			};

			NazaraSignal(OnEntityCreated, NetworkSyncSystem* /*emitter*/, const EntityCreation& /*event*/);
			NazaraSignal(OnEntityDeleted, NetworkSyncSystem* /*emitter*/, const EntityDestruction& /*event*/);
			NazaraSignal(OnEntityPlayAnimation, NetworkSyncSystem* /*emitter*/, const EntityPlayAnimation& /*event*/);
			NazaraSignal(OnEntitiesInputUpdate, NetworkSyncSystem* /*emitter*/, const EntityInputs* /*events*/, std::size_t /*entityCount*/);
			NazaraSignal(OnEntitiesHealthUpdate, NetworkSyncSystem* /*emitter*/, const EntityHealth* /*events*/, std::size_t /*entityCount*/);

		private:
			void CreateEntity(EntityCreation& creationEvent, Ndk::Entity* entity) const;
			void DeleteEntity(EntityDestruction& deleteEvent, Ndk::Entity* entity) const;

			void OnEntityAdded(Ndk::Entity* entity) override;
			void OnEntityRemoved(Ndk::Entity* entity) override;
			void OnUpdate(float elapsedTime) override;

			struct EntitySlots
			{
				NazaraSlot(AnimationComponent, OnAnimationStart, onAnimationStart);
				NazaraSlot(HealthComponent, OnHealthChange, onHealthChange);
				NazaraSlot(InputComponent, OnInputUpdate, onInputUpdate);
			};

			tsl::hopscotch_map<Ndk::EntityId, EntitySlots> m_entitySlots;

			Ndk::EntityList m_inputUpdateEntities;
			Ndk::EntityList m_healthUpdateEntities;
			Ndk::EntityList m_physicsEntities;
			Ndk::EntityList m_staticEntities;
			mutable std::vector<EntityCreation> m_creationEvents;
			mutable std::vector<EntityDestruction> m_destructionEvents;
			std::vector<EntityHealth> m_healthEvents;
			std::vector<EntityInputs> m_inputEvents;
			mutable std::vector<EntityMovement> m_movementEvents;
	};
}

#include <Shared/Systems/NetworkSyncSystem.inl>

#endif
