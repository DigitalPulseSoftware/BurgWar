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
#include <iostream>

namespace bw
{
	TerrainLayer::TerrainLayer(BurgApp& app, Match& match, const MapData::Layer& layerData, const Map::Layer& layerData2, float tileSize)
	{
		m_world.AddSystem<AnimationSystem>(app);
		m_world.AddSystem<NetworkSyncSystem>();
		m_world.AddSystem<PlayerMovementSystem>();
		m_world.AddSystem<TickCallbackSystem>();

		Ndk::PhysicsSystem2D& physics = m_world.GetSystem<Ndk::PhysicsSystem2D>();
		physics.SetGravity(Nz::Vector2f(0.f, 9.81f * 128.f));
		physics.SetMaximumUpdateRate(30.f);
		/*physics.SetMaxStepCount(3);
		physics.SetStepSize(1.f / 40.f);*/


		// Create map collider
		const Ndk::EntityHandle& map = m_world.CreateEntity();

		std::vector<Nz::Collider2DRef> colliders;
		for (std::size_t y = 0; y < layerData.height; ++y)
		{
			for (std::size_t x = 0; x < layerData.width; ++x)
			{
				if (layerData.tiles[y * layerData.width + x] != 0)
				{
					std::size_t startX = x++;

					while (x < layerData.width && layerData.tiles[y * layerData.width + x] != 0) ++x;

					std::cout << "[server] " << Nz::Rectf(startX * tileSize, y * tileSize, (x - startX) * tileSize, tileSize) << std::endl;
					colliders.emplace_back(Nz::BoxCollider2D::New(Nz::Rectf(startX * tileSize, y * tileSize, (x - startX) * tileSize, tileSize)));
				}
			}
		}

		Nz::CompoundCollider2DRef collider = Nz::CompoundCollider2D::New(std::move(colliders));
		collider->SetCollisionId(0);

		map->AddComponent<Ndk::NodeComponent>().SetPosition(0.f, 0.f);
		map->AddComponent<Ndk::CollisionComponent2D>(collider);
		auto& theColliderPhys = map->AddComponent<Ndk::PhysicsComponent2D>();
		theColliderPhys.SetMass(0.f);
		theColliderPhys.SetFriction(1.f);

		auto& entityStore = match.GetEntityStore();
		for (const Map::Entity& entityData : layerData2.entities)
		{
			std::size_t entityTypeIndex = entityStore.GetElementIndex("entity_" + entityData.entityType);
			if (entityTypeIndex == entityStore.InvalidIndex)
			{
				std::cerr << "Unknown entity type " + entityData.entityType << std::endl;
				continue;
			}

			entityStore.InstantiateEntity(m_world, entityTypeIndex, entityData.position, entityData.rotation, entityData.properties);
		}
	}

	void TerrainLayer::Update(float elapsedTime)
	{
		m_world.Update(elapsedTime);
	}
}
