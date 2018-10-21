// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_SHARED_SYSTEMS_NETWORKSYNCSYSTEM_HPP
#define BURGWAR_SHARED_SYSTEMS_NETWORKSYNCSYSTEM_HPP

#include <Nazara/Math/Angle.hpp>
#include <Nazara/Math/Vector2.hpp>
#include <NDK/System.hpp>
#include <optional>
#include <variant>
#include <vector>

namespace bw
{
	class NetworkSyncSystem : public Ndk::System<NetworkSyncSystem>
	{
		public:
			struct Event;

			NetworkSyncSystem();
			~NetworkSyncSystem() = default;

			inline void ClearEvents();

			inline const std::vector<Event>& GetEvents() const;

			static Ndk::SystemIndex systemIndex;

			struct Event
			{
				struct EntityCreation
				{
					Nz::RadianAnglef angularVelocity;
					Nz::RadianAnglef rotation;
					Nz::Vector2f position;
					Nz::Vector2f linearVelocity;
					bool hasPlayerMovement;
				};

				struct EntityDestruction
				{
				};

				struct EntityMovement
				{
					struct PlayerMovementData
					{
						bool isAirControlling;
						bool isFacingRight;
					};

					Nz::RadianAnglef angularVelocity;
					Nz::RadianAnglef rotation;
					Nz::Vector2f position;
					Nz::Vector2f linearVelocity;
					std::optional<PlayerMovementData> playerMovement;
				};

				std::variant<EntityCreation, EntityDestruction, EntityMovement> eventData;
				Ndk::EntityId id;
			};

		private:
			void OnEntityAdded(Ndk::Entity* entity) override;
			void OnEntityRemoved(Ndk::Entity* entity) override;
			void OnUpdate(float elapsedTime) override;

			Ndk::EntityList m_physicsEntities;
			Ndk::EntityList m_staticEntities;
			std::vector<Event> m_events;
	};
}

#include <Shared/Systems/NetworkSyncSystem.inl>

#endif
