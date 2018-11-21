// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_SERVER_CLIENTVISIBILITY_HPP
#define BURGWAR_SERVER_CLIENTVISIBILITY_HPP

#include <Nazara/Core/Bitset.hpp>
#include <Nazara/Core/Signal.hpp>
#include <Shared/Protocol/Packets.hpp>
#include <Shared/Components/HealthComponent.hpp>
#include <Shared/Systems/NetworkSyncSystem.hpp>
#include <limits>

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

			void Update(float elapsedTime);
			void UpdateLayer(std::size_t layerIndex);

			static constexpr std::size_t NoLayer = std::numeric_limits<std::size_t>::max();

		private:
			void HandleEntityCreation(Packets::CreateEntities& createPacket, const NetworkSyncSystem::EntityCreation& eventData);
			void HandleEntityDestruction(Packets::DeleteEntities& deletePacket, const NetworkSyncSystem::EntityDestruction& eventData);
			void HandleEntityHealthUpdate(Packets::HealthUpdate& healthPacket, const NetworkSyncSystem::EntityHealth& eventData);

			NazaraSlot(NetworkSyncSystem, OnEntityCreated, m_onEntityCreatedSlot);
			NazaraSlot(NetworkSyncSystem, OnEntityDeleted, m_onEntityDeletedSlot);
			NazaraSlot(NetworkSyncSystem, OnEntityPlayAnimation, m_onEntityPlayAnimation);
			NazaraSlot(NetworkSyncSystem, OnEntitiesHealthUpdate, m_onEntitiesHealthUpdate);

			std::size_t m_activeLayer;
			Nz::Bitset<Nz::UInt64> m_visibleEntities;
			Match& m_match;
			MatchClientSession& m_session;
			float m_entityMovementSendInterval;
			float m_entityMovementSendTimer;
	};
}

#include <Shared/MatchClientVisibility.inl>

#endif