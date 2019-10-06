// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_LOCALMATCHPREDICTION_HPP
#define BURGWAR_CLIENTLIB_LOCALMATCHPREDICTION_HPP

#include <Nazara/Core/Bitset.hpp>
#include <Nazara/Math/Vector2.hpp>
#include <NDK/EntityOwner.hpp>
#include <NDK/World.hpp>
#include <tsl/hopscotch_map.h>
#include <optional>
#include <vector>

namespace bw
{
	class LocalMatch;

	class LocalMatchPrediction
	{
		friend class ClientSession;

		public:
			LocalMatchPrediction(float tickDuration);
			~LocalMatchPrediction() = default;

			inline void DeleteEntity(Ndk::EntityId entityId);
			inline void DeleteUnregisteredEntities();

			inline const Ndk::EntityHandle& GetEntity(Ndk::EntityId entityId);

			inline bool IsRegistered(Ndk::EntityId entityId) const;

			inline void RegisterForPrediction(const Ndk::EntityHandle& entity);
			void RegisterForPrediction(const Ndk::EntityHandle& entity, const std::function<void(const Ndk::EntityHandle& source, const Ndk::EntityHandle& target)>& syncFunc);
			inline void RegisterForPrediction(const Ndk::EntityHandle& entity, const std::function<void(const Ndk::EntityHandle& entity)>& constructor);
			void RegisterForPrediction(const Ndk::EntityHandle& entity, const std::function<void(const Ndk::EntityHandle& entity)>& constructor, const std::function<void(const Ndk::EntityHandle& source, const Ndk::EntityHandle& target)>& syncFunc);

			inline void Tick();

			static void SynchronizeEntity(const Ndk::EntityHandle& source, const Ndk::EntityHandle& target);

		private:
			const Ndk::EntityHandle& CreateReconciliationEntity(const Ndk::EntityHandle& serverEntity);

			tsl::hopscotch_map<Ndk::EntityId, Ndk::EntityOwner> m_entities;
			Nz::Bitset<Nz::UInt64> m_registeredEntities;
			Ndk::World m_world;
			float m_tickDuration;
	};
}

#include <ClientLib/LocalMatchPrediction.inl>

#endif