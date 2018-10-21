// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/TerrainLayer.hpp>
#include <Shared/Components/NetworkSyncComponent.hpp>
#include <Shared/Components/PlayerControlledComponent.hpp>
#include <Shared/Components/PlayerMovementComponent.hpp>
#include <Shared/Systems/NetworkSyncSystem.hpp>
#include <Shared/Systems/PlayerControlledSystem.hpp>
#include <Shared/Systems/PlayerMovementSystem.hpp>
#include <Nazara/Physics2D/Arbiter2D.hpp>
#include <NDK/Components.hpp>
#include <NDK/Systems.hpp>
#include <iostream>

namespace bw
{
	TerrainLayer::TerrainLayer(const MapData::Layer& layerData, float tileSize)
	{
		m_world.AddSystem<NetworkSyncSystem>();
		m_world.AddSystem<PlayerControlledSystem>();
		m_world.AddSystem<PlayerMovementSystem>();

		Ndk::PhysicsSystem2D& physics = m_world.GetSystem<Ndk::PhysicsSystem2D>();
		physics.SetGravity(Nz::Vector2f(0.f, 9.81f * 128.f));

		Ndk::PhysicsSystem2D::Callback cb;
		cb.endCallback = [&](Ndk::PhysicsSystem2D& world, Nz::Arbiter2D& arbiter, const Ndk::EntityHandle& bodyA, const Ndk::EntityHandle& bodyB, void* userdata)
		{
			if (bodyB->HasComponent<PlayerControlledComponent>())
				bodyB->GetComponent<PlayerControlledComponent>().UpdateGroundState(false);
		};
		cb.postSolveCallback = [&](Ndk::PhysicsSystem2D& world, Nz::Arbiter2D& arbiter, const Ndk::EntityHandle& bodyA, const Ndk::EntityHandle& bodyB, void* userdata)
		{
			if (bodyB->HasComponent<PlayerControlledComponent>())
				bodyB->GetComponent<PlayerControlledComponent>().UpdateGroundState(true);
		};
		cb.userdata = nullptr;
		physics.RegisterCallbacks(0, 1, std::move(cb));

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


		Nz::MaterialRef burgerMat = Nz::Material::New("Translucent2D");
		burgerMat->SetDiffuseMap("../resources/burger.png");
		auto& sampler = burgerMat->GetDiffuseSampler();
		sampler.SetFilterMode(Nz::SamplerFilter_Nearest);

		Nz::Vector2f burgerSize = Nz::Vector2f(Nz::Vector3f(burgerMat->GetDiffuseMap()->GetSize())) / 2.f;

		auto burgerBox = Nz::BoxCollider2D::New(Nz::Rectf(-burgerSize.x / 2.f, -burgerSize.y, burgerSize.x, burgerSize.y - 3.f));
		burgerBox->SetCollisionId(1);

		Ndk::EntityHandle burger = m_world.CreateEntity();
		burger->AddComponent<NetworkSyncComponent>();
		burger->AddComponent<PlayerControlledComponent>();
		burger->AddComponent<PlayerMovementComponent>();
		burger->AddComponent<Ndk::NodeComponent>().SetPosition(640.f, 100.f);
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
