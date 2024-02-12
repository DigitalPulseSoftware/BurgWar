// Copyright (C) 2020 Jérôme Leclercq
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
#include <Nazara/Physics2D/PhysArbiter2D.hpp>

namespace bw
{
	TerrainLayer::TerrainLayer(Match& match, LayerIndex layerIndex, const Map::Layer& layerData) :
	SharedLayer(match, layerIndex),
	m_mapLayer(layerData)
	{
		Nz::EnttSystemGraph& systemGraph = GetSystemGraph();
		systemGraph.AddSystem<NetworkSyncSystem>(*this);

		ResetEntities();
	}

	Match& TerrainLayer::GetMatch()
	{
		return static_cast<Match&>(SharedLayer::GetMatch());
	}

	void TerrainLayer::ResetEntities()
	{
		Match& match = GetMatch();
		
		entt::registry& world = GetWorld();
		world.clear();

		auto& entityStore = match.GetEntityStore();
		for (const Map::Entity& entityData : m_mapLayer.entities)
		{
			std::size_t entityTypeIndex = entityStore.GetElementIndex(entityData.entityType);
			if (entityTypeIndex == entityStore.InvalidIndex)
			{
				bwLog(match.GetLogger(), LogLevel::Error, "Unknown entity type {0}", entityData.entityType);
				continue;
			}

			try
			{
				entt::handle entity = entityStore.CreateEntity(*this, entityTypeIndex, entityData.uniqueId, entityData.position, entityData.rotation, entityData.properties);
				if (entity)
					match.RegisterEntity(entityData.uniqueId, entity);
			}
			catch (const std::exception& e)
			{
				bwLog(match.GetLogger(), LogLevel::Error, "Failed to instantiate entity {0}: {1}", entityData.entityType, e.what());
			}
		}
	}

	void TerrainLayer::InitializeEntities()
	{
		auto& entityStore = GetMatch().GetEntityStore();
		entt::registry& registry = GetWorld();
		for (auto [entity] : registry.storage<entt::entity>().each())
		{
			if (!entityStore.InitializeEntity(entt::handle(registry, entity)))
				registry.destroy(entity);
		}
	}
}
