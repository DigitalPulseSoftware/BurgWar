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
#include <Thirdparty/tsl/hopscotch_map.h>
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

			inline void DeleteEntity(Nz::UInt64 layerEntityId);
			inline void DeleteUnregisteredEntities();

			inline const Ndk::EntityHandle& GetEntity(Nz::UInt64 layerEntityId);

			inline bool IsRegistered(Nz::UInt64 layerEntityId) const;

			inline void RegisterForPrediction(Nz::UInt64 layerEntityId, const Ndk::EntityHandle& entity);
			void RegisterForPrediction(Nz::UInt64 layerEntityId, const Ndk::EntityHandle& entity, const std::function<void(const Ndk::EntityHandle& source, const Ndk::EntityHandle& target)>& syncFunc);
			inline void RegisterForPrediction(Nz::UInt64 layerEntityId, const Ndk::EntityHandle& entity, const std::function<void(const Ndk::EntityHandle& entity)>& constructor);
			void RegisterForPrediction(Nz::UInt64 layerEntityId, const Ndk::EntityHandle& entity, const std::function<void(const Ndk::EntityHandle& entity)>& constructor, const std::function<void(const Ndk::EntityHandle& source, const Ndk::EntityHandle& target)>& syncFunc);

			inline void Tick();

			static void SynchronizeEntity(const Ndk::EntityHandle& source, const Ndk::EntityHandle& target);

		private:
			const Ndk::EntityHandle& CreateReconciliationEntity(Nz::UInt64 layerEntityId, const Ndk::EntityHandle& serverEntity);

			struct EntityData
			{
				Ndk::EntityOwner entity;
				bool isRegistered = true;
			};

			tsl::hopscotch_map<Nz::UInt64, EntityData> m_entities;
			Ndk::World m_world;
			float m_tickDuration;
	};
}

#include <ClientLib/LocalMatchPrediction.inl>

#endif