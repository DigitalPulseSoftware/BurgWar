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
		world.AddSystem<NetworkSyncSystem>(*this);

		auto& entityStore = match.GetEntityStore();
		for (const Map::Entity& entityData : layerData.entities)
		{
			std::size_t entityTypeIndex = entityStore.GetElementIndex(entityData.entityType);
			if (entityTypeIndex == entityStore.InvalidIndex)
			{
				bwLog(match.GetLogger(), LogLevel::Error, "Unknown entity type {0}", entityData.entityType);
				continue;
			}

			try
			{
				const Ndk::EntityHandle& entity = entityStore.InstantiateEntity(*this, entityTypeIndex, entityData.uniqueId, entityData.position, entityData.rotation, entityData.properties);
				if (entity)
					match.RegisterEntity(entityData.uniqueId, entity);
			}
			catch (const std::exception& e)
			{
				bwLog(match.GetLogger(), LogLevel::Error, "Failed to instantiate entity {0}: {1}", entityData.entityType, e.what());
			}
		}
	}

	Match& TerrainLayer::GetMatch()
	{
		return static_cast<Match&>(SharedLayer::GetMatch());
	}
}
