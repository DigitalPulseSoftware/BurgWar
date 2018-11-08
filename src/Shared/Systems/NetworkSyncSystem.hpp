// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_SHARED_SYSTEMS_NETWORKSYNCSYSTEM_HPP
#define BURGWAR_SHARED_SYSTEMS_NETWORKSYNCSYSTEM_HPP

#include <Nazara/Core/Signal.hpp>
#include <Nazara/Math/Angle.hpp>
#include <Nazara/Math/Vector2.hpp>
#include <NDK/System.hpp>
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

			struct PlayerMovementData
			{
				bool isAirControlling;
				bool isFacingRight;
			};

			struct EntityCreation
			{
				Ndk::EntityId id;
				Nz::RadianAnglef angularVelocity;
				Nz::RadianAnglef rotation;
				Nz::Vector2f position;
				Nz::Vector2f linearVelocity;
				std::optional<PlayerMovementData> playerMovement;
				std::string entityClass;
			};

			struct EntityDestruction
			{
				Ndk::EntityId id;
			};

			struct EntityMovement
			{
				Ndk::EntityId id;
				Nz::RadianAnglef angularVelocity;
				Nz::RadianAnglef rotation;
				Nz::Vector2f position;
				Nz::Vector2f linearVelocity;
				std::optional<PlayerMovementData> playerMovement;
			};

			NazaraSignal(OnEntityCreated, NetworkSyncSystem* /*emitter*/, const EntityCreation& /*event*/);
			NazaraSignal(OnEntityDeleted, NetworkSyncSystem* /*emitter*/, const EntityDestruction& /*event*/);

		private:
			void CreateEntity(EntityCreation& creationEvent, Ndk::Entity* entity) const;
			void DeleteEntity(EntityDestruction& deleteEvent, Ndk::Entity* entity) const;

			void OnEntityAdded(Ndk::Entity* entity) override;
			void OnEntityRemoved(Ndk::Entity* entity) override;
			void OnUpdate(float elapsedTime) override;

			Ndk::EntityList m_physicsEntities;
			Ndk::EntityList m_staticEntities;
			mutable std::vector<EntityCreation> m_creationEvents;
			mutable std::vector<EntityDestruction> m_destructionEvents;
			mutable std::vector<EntityMovement> m_movementEvents;
	};
}

#include <Shared/Systems/NetworkSyncSystem.inl>

#endif
