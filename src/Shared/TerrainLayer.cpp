// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/TerrainLayer.hpp>
#include <Shared/Components/NetworkSyncComponent.hpp>
#include <Shared/Systems/NetworkSyncSystem.hpp>
#include <NDK/Components.hpp>
#include <NDK/Systems.hpp>

namespace bw
{
	TerrainLayer::TerrainLayer()
	{
		m_world.AddSystem<NetworkSyncSystem>();

		Ndk::PhysicsSystem2D& physics = m_world.GetSystem<Ndk::PhysicsSystem2D>();
		physics.SetGravity(Nz::Vector2f(0.f, 9.81f * 128.f));

		// Create map layer

		float tileSize = 64.f;
		float mapSize = 12.f;

		const Ndk::EntityHandle& map = m_world.CreateEntity();

		Nz::BoxCollider2DRef collider = Nz::BoxCollider2D::New(Nz::Rectf(-2000.f, 7 * tileSize, tileSize * mapSize + 4000.f, 2.f * tileSize));
		collider->SetCollisionId(0);

		map->AddComponent<Ndk::NodeComponent>().SetPosition(0.f, 0.f);
		map->AddComponent<Ndk::CollisionComponent2D>(collider);

		auto& mapPhys = map->AddComponent<Ndk::PhysicsComponent2D>();
		mapPhys.SetMass(0.f);
		mapPhys.SetFriction(1.f);


		Nz::MaterialRef burgerMat = Nz::Material::New("Translucent2D");
		burgerMat->SetDiffuseMap("../resources/burger.png");
		auto& sampler = burgerMat->GetDiffuseSampler();
		sampler.SetFilterMode(Nz::SamplerFilter_Nearest);

		Nz::Vector2f burgerSize = Nz::Vector2f(Nz::Vector3f(burgerMat->GetDiffuseMap()->GetSize()));

		auto burgerBox = Nz::BoxCollider2D::New(Nz::Rectf(0.f, -burgerSize.y, burgerSize.x, burgerSize.y - 3.f));
		burgerBox->SetCollisionId(1);

		Ndk::EntityHandle burger = m_world.CreateEntity();
		burger->AddComponent<NetworkSyncComponent>();
		burger->AddComponent<Ndk::NodeComponent>().SetPosition(300.f, 100.f);
		burger->AddComponent<Ndk::CollisionComponent2D>(burgerBox);
		auto& burgerPhys = burger->AddComponent<Ndk::PhysicsComponent2D>();
		burgerPhys.SetMass(300);
		burgerPhys.SetFriction(10.f);
		burgerPhys.SetMomentOfInertia(std::numeric_limits<float>::infinity());
	}

	void TerrainLayer::Update(float elapsedTime)
	{
		m_world.Update(elapsedTime);
	}
}
