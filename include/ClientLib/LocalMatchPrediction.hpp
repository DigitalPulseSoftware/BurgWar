// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_LOCALMATCHPREDICTION_HPP
#define BURGWAR_CLIENTLIB_LOCALMATCHPREDICTION_HPP

#include <Nazara/Math/Vector2.hpp>
#include <NDK/EntityOwner.hpp>
#include <NDK/World.hpp>
#include <tsl/hopscotch_map.h>
#include <optional>
#include <vector>

namespace bw
{
	class BurgApp;
	class ClientGamemode;
	class InputController;
	class LocalMatch;
	class VirtualDirectory;

	class LocalMatchPrediction
	{
		friend class ClientSession;

		public:
			struct PredictedInput;

			LocalMatchPrediction(LocalMatch& match);
			~LocalMatchPrediction() = default;


			void OnEntityDeletion(Ndk::EntityId entityId);

			void PushInput(PredictedInput input);

			void RegisterEntitiesForPrediction(const Nz::Bitset<>& entities, const std::function<const Ndk::EntityHandle&(std::size_t)>& retrieveEntity);
			void RegisterForPrediction(Ndk::EntityId entityId, const Ndk::EntityHandle& entity);
			void RegisterForPrediction(Ndk::EntityId entityId, const Ndk::EntityHandle& entity, const std::function<void(const Ndk::EntityHandle& entity)>& constructor);

			struct PredictedInput
			{
				struct MovementData
				{
					Nz::Vector2f surfaceVelocity;
					bool wasJumping;
					bool isOnGround;
					float jumpTime;
					float friction;
				};

				struct PlayerData
				{
					InputData input;
					std::optional<MovementData> movement;
				};

				Nz::UInt16 serverTick;
				std::vector<PlayerData> inputs;
			};

		private:
			tsl::hopscotch_map<Ndk::EntityId, Ndk::EntityOwner> m_reconciliationEntities;
			std::vector<PredictedInput> m_predictedInputs;
			Ndk::World m_world;
	};
}

#include <ClientLib/LocalMatchPrediction.inl>

#endif