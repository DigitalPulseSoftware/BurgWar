// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/TerrainLayer.hpp>
#include <NDK/Components.hpp>
#include <NDK/Systems.hpp>

namespace bw
{
	TerrainLayer::TerrainLayer()
	{
		// Create map layer

		float tileSize = 64.f;
		float mapSize = 12.f;

		const Ndk::EntityHandle& map = m_world.CreateEntity();

		Nz::BoxCollider2DRef collider = Nz::BoxCollider2D::New(Nz::Rectf(-2000.f, 8 * tileSize, tileSize * mapSize + 4000.f, 2.f * tileSize));
		collider->SetCollisionId(0);

		map->AddComponent<Ndk::NodeComponent>().SetPosition(0.f, 0.f);
		map->AddComponent<Ndk::CollisionComponent2D>(collider);

		auto& mapPhys = map->AddComponent<Ndk::PhysicsComponent2D>();
		mapPhys.SetMass(0.f);
		mapPhys.SetFriction(1.f);
	}

	void TerrainLayer::Update(float elapsedTime)
	{
		m_world.Update(elapsedTime);
	}
}
