// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <GameLibShared/TerrainLayer.hpp>
#include <GameLibShared/Components/NetworkSyncComponent.hpp>
#include <GameLibShared/Components/PlayerControlledComponent.hpp>
#include <GameLibShared/Components/PlayerMovementComponent.hpp>
#include <GameLibShared/Systems/AnimationSystem.hpp>
#include <GameLibShared/Systems/NetworkSyncSystem.hpp>
#include <GameLibShared/Systems/PlayerMovementSystem.hpp>
#include <GameLibShared/Systems/TickCallbackSystem.hpp>
#include <Nazara/Physics2D/Arbiter2D.hpp>
#include <NDK/Components.hpp>
#include <NDK/Systems.hpp>
#include <iostream>

namespace bw
{
	TerrainLayer::TerrainLayer(BurgApp& app, const MapData::Layer& layerData, float tileSize)
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
	}

	void TerrainLayer::Update(float elapsedTime)
	{
		m_world.Update(elapsedTime);
	}
}
