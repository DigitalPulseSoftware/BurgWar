// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/TerrainLayer.hpp>
#include <CoreLib/Match.hpp>
#include <CoreLib/Components/NetworkSyncComponent.hpp>
#include <CoreLib/Components/PlayerControlledComponent.hpp>
#include <CoreLib/Components/PlayerMovementComponent.hpp>
#include <CoreLib/Systems/AnimationSystem.hpp>
#include <CoreLib/Systems/NetworkSyncSystem.hpp>
#include <CoreLib/Systems/PlayerMovementSystem.hpp>
#include <CoreLib/Systems/TickCallbackSystem.hpp>
#include <Nazara/Physics2D/Arbiter2D.hpp>
#include <NDK/Components.hpp>
#include <NDK/Systems.hpp>

namespace bw
{
	TerrainLayer::TerrainLayer(Match& match, LayerIndex layerIndex, const Map::Layer& layerData) :
	SharedLayer(match, layerIndex)
	{
		Ndk::World& world = GetWorld();
		world.AddSystem<NetworkSyncSystem>(static_cast<Nz::UInt16>(layerIndex));

		auto& entityStore = match.GetEntityStore();
		for (const Map::Entity& entityData : layerData.entities)
		{
			std::size_t entityTypeIndex = entityStore.GetElementIndex(entityData.entityType);
			if (entityTypeIndex == entityStore.InvalidIndex)
			{
				bwLog(match.GetLogger(), LogLevel::Error, "Unknown entity type {0}", entityData.entityType);
				continue;
			}

			entityStore.InstantiateEntity(*this, entityTypeIndex, entityData.position, entityData.rotation, entityData.properties);
		}
	}

	Match& TerrainLayer::GetMatch()
	{
		return static_cast<Match&>(SharedLayer::GetMatch());
	}
}
