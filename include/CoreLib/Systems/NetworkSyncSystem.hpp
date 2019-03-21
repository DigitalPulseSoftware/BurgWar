// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SYSTEMS_NETWORKSYNCSYSTEM_HPP
#define BURGWAR_CORELIB_SYSTEMS_NETWORKSYNCSYSTEM_HPP

#include <CoreLib/Components/AnimationComponent.hpp>
#include <CoreLib/Components/HealthComponent.hpp>
#include <CoreLib/Components/InputComponent.hpp>
#include <CoreLib/Components/NetworkSyncComponent.hpp>
#include <CoreLib/Scripting/ScriptedElement.hpp>
#include <Nazara/Core/Signal.hpp>
#include <Nazara/Math/Angle.hpp>
#include <Nazara/Math/Vector2.hpp>
#include <NDK/System.hpp>
#include <tsl/hopscotch_map.h>
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
				Ndk::EntityId entityId;
				Nz::RadianAnglef rotation;
				Nz::Vector2f position;
				std::optional<Ndk::EntityId> parent;
				std::optional<HealthProperties> healthProperties;
				std::optional<InputData> inputs;
				std::optional<PlayerMovementData> playerMovement;
				std::optional<PhysicsProperties> physicsProperties;
				std::string entityClass;
				tsl::hopscotch_map<std::string /*key*/, EntityProperty> properties;
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
				InputData inputs;
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
			NazaraSignal(OnEntityDeleted, NetworkSyncSystem* /*emitter*/, const EntityDestruction& /*event*/);
			NazaraSignal(OnEntityPlayAnimation, NetworkSyncSystem* /*emitter*/, const EntityPlayAnimation& /*event*/);
			NazaraSignal(OnEntityInvalidated, NetworkSyncSystem* /*emitter*/, const EntityMovement& /*event*/);
			NazaraSignal(OnEntitiesInputUpdate, NetworkSyncSystem* /*emitter*/, const EntityInputs* /*events*/, std::size_t /*entityCount*/);
			NazaraSignal(OnEntitiesHealthUpdate, NetworkSyncSystem* /*emitter*/, const EntityHealth* /*events*/, std::size_t /*entityCount*/);

		private:
			void BuildEvent(EntityCreation& creationEvent, Ndk::Entity* entity) const;
			void BuildEvent(EntityDestruction& deleteEvent, Ndk::Entity* entity) const;
			void BuildEvent(EntityMovement &movementEvent, Ndk::Entity* entity) const;

			void OnEntityAdded(Ndk::Entity* entity) override;
			void OnEntityRemoved(Ndk::Entity* entity) override;
			void OnUpdate(float elapsedTime) override;

			struct EntitySlots
			{
				NazaraSlot(AnimationComponent, OnAnimationStart, onAnimationStart);
				NazaraSlot(HealthComponent, OnHealthChange, onHealthChange);
				NazaraSlot(InputComponent, OnInputUpdate, onInputUpdate);
				NazaraSlot(NetworkSyncComponent, OnInvalidated, onInvalidated);
			};

			tsl::hopscotch_map<Ndk::EntityId, EntitySlots> m_entitySlots;

			Ndk::EntityList m_inputUpdateEntities;
			Ndk::EntityList m_healthUpdateEntities;
			Ndk::EntityList m_physicsEntities;
			Ndk::EntityList m_staticEntities;
			Ndk::EntityList m_invalidatedEntities;
			mutable std::vector<EntityCreation> m_creationEvents;
			mutable std::vector<EntityDestruction> m_destructionEvents;
			std::vector<EntityHealth> m_healthEvents;
			std::vector<EntityInputs> m_inputEvents;
			mutable std::vector<EntityMovement> m_movementEvents;
	};
}

#include <CoreLib/Systems/NetworkSyncSystem.inl>

#endif
