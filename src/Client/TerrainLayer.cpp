// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/TerrainLayer.hpp>
#include <Client/BurgApp.hpp>
#include <Client/Components/PlayerControlledComponent.hpp>
#include <Client/Systems/PlayerControlledSystem.hpp>
#include <Nazara/Graphics/ColorBackground.hpp>
#include <Nazara/Graphics/Sprite.hpp>
#include <Nazara/Graphics/TileMap.hpp>
#include <NDK/Components.hpp>
#include <NDK/Systems.hpp>

namespace bw
{
	TerrainLayer::TerrainLayer(BurgApp& app)
	{
		m_world.AddSystem<PlayerControlledSystem>();

		// Create map layer

		Ndk::RenderSystem& renderSystem = m_world.GetSystem<Ndk::RenderSystem>();
		renderSystem.SetGlobalUp(Nz::Vector3f::Down());
		renderSystem.SetDefaultBackground(Nz::ColorBackground::New(Nz::Color(135, 206, 235)));

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


		m_camera = m_world.CreateEntity();
		m_camera->AddComponent<Ndk::NodeComponent>();

		Ndk::CameraComponent& viewer = m_camera->AddComponent<Ndk::CameraComponent>();
		viewer.SetTarget(&(app.GetMainWindow()));
		viewer.SetProjectionType(Nz::ProjectionType_Orthogonal);

		Nz::MaterialRef dirtMat = Nz::Material::New();
		dirtMat->SetDiffuseMap("../resources/dirt.png");

		Nz::MaterialRef grassMat = Nz::Material::New("Translucent2D");
		grassMat->SetDiffuseMap("../resources/grass.png");

		float grassScale = 64.f / 615.f;

		Nz::SpriteRef grassSprite = Nz::Sprite::New();
		grassSprite->SetMaterial(grassMat);
		grassSprite->SetSize(grassSprite->GetSize() * grassScale);
		grassSprite->SetOrigin(Nz::Vector3f(0.f, 8.f, 0.f));

		Nz::TileMapRef tileMap = Nz::TileMap::New(Nz::Vector2ui(15, 10), Nz::Vector2f(64.f, 64.f));
		tileMap->SetMaterial(0, dirtMat);

		const Ndk::EntityHandle& map = m_world.CreateEntity();
		auto& worldGfx = map->AddComponent<Ndk::GraphicsComponent>();

		for (std::size_t i = 0; i < tileMap->GetMapSize().x; ++i)
		{
			tileMap->EnableTile(Nz::Vector2ui(i, 8), Nz::Rectf(0.f, 0.f, 1.f, 1.f));
			tileMap->EnableTile(Nz::Vector2ui(i, 9), Nz::Rectf(0.f, 0.f, 1.f, 1.f));

			Nz::Vector3f pos = Nz::Vector2f(i, 8) * tileMap->GetTileSize();

			worldGfx.Attach(grassSprite, Nz::Matrix4f::Translate(pos), 5);
		}

		worldGfx.Attach(tileMap);

		Nz::BoxCollider2DRef collider = Nz::BoxCollider2D::New(Nz::Rectf(-2000.f, 8 * 64.f, tileMap->GetTileSize().x * tileMap->GetMapSize().x + 4000.f, 2.f * 64.f));
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
