// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/LocalMatch.hpp>
#include <Client/BurgApp.hpp>
#include <Nazara/Graphics/ColorBackground.hpp>
#include <Nazara/Graphics/TileMap.hpp>
#include <Nazara/Renderer/DebugDrawer.hpp>
#include <NDK/Components.hpp>
#include <NDK/Systems.hpp>
#include <cassert>
#include <iostream>

namespace bw
{
	LocalMatch::LocalMatch(BurgApp& burgApp, const Packets::MatchData& matchData) :
	m_application(burgApp)
	{
		Ndk::RenderSystem& renderSystem = m_world.GetSystem<Ndk::RenderSystem>();
		renderSystem.SetGlobalUp(Nz::Vector3f::Down());
		renderSystem.SetDefaultBackground(Nz::ColorBackground::New(matchData.backgroundColor));

		Ndk::PhysicsSystem2D& physics = m_world.GetSystem<Ndk::PhysicsSystem2D>();
		physics.SetGravity(Nz::Vector2f(0.f, 9.81f * 128.f));

		Ndk::EntityHandle camera = m_world.CreateEntity();
		camera->AddComponent<Ndk::NodeComponent>();

		Ndk::CameraComponent& viewer = camera->AddComponent<Ndk::CameraComponent>();
		viewer.SetTarget(&(m_application.GetMainWindow()));
		viewer.SetProjectionType(Nz::ProjectionType_Orthogonal);

		Nz::MaterialRef dirtMat = Nz::Material::New();
		dirtMat->SetDiffuseMap("../resources/dirt.png");

		Nz::MaterialRef grassMat = Nz::Material::New("Translucent2D");
		grassMat->SetDiffuseMap("../resources/grass.png");

		float grassScale = matchData.tileSize / grassMat->GetDiffuseMap()->GetWidth();

		Nz::SpriteRef grassSprite = Nz::Sprite::New();
		grassSprite->SetMaterial(grassMat);
		grassSprite->SetSize(grassSprite->GetSize() * grassScale);
		grassSprite->SetOrigin(Nz::Vector3f(0.f, 8.f, 0.f));

		Nz::TileMapRef tileMap = Nz::TileMap::New(Nz::Vector2ui(matchData.width, matchData.height), Nz::Vector2f(matchData.tileSize, matchData.tileSize));
		tileMap->SetMaterial(0, dirtMat);

		Ndk::EntityHandle theCollider = m_world.CreateEntity();
		auto& worldGfx = theCollider->AddComponent<Ndk::GraphicsComponent>();

		for (std::size_t y = 0; y < matchData.height; ++y)
		{
			for (std::size_t x = 0; x < matchData.width; ++x)
			{
				switch (matchData.tiles[y * matchData.width + x])
				{
					case 0: //< Air
						break;

					case 2: //< Grass
					{
						Nz::Vector3f pos = Nz::Vector2f(x, y) * tileMap->GetTileSize();

						worldGfx.Attach(grassSprite, Nz::Matrix4f::Translate(pos), 5);

						[[fallthrough]];
					}

					case 1: //< Dirt
					{
						tileMap->EnableTile(Nz::Vector2ui(x, y), Nz::Rectf(0.f, 0.f, 1.f, 1.f));
						break;
					}
				}
			}
		}

		worldGfx.Attach(tileMap);

		std::vector<Nz::Collider2DRef> colliders;
		for (std::size_t y = 0; y < matchData.height; ++y)
		{
			for (std::size_t x = 0; x < matchData.width; ++x)
			{
				if (matchData.tiles[y * matchData.width + x] != 0)
				{
					std::size_t startX = x++;

					while (x < matchData.width && matchData.tiles[y * matchData.width + x] != 0) ++x;

					colliders.emplace_back(Nz::BoxCollider2D::New(Nz::Rectf(startX * tileMap->GetTileSize().x, y * tileMap->GetTileSize().y, (x - startX) * tileMap->GetTileSize().x, tileMap->GetTileSize().y)));
				}
			}
		}

		Nz::CompoundCollider2DRef collider = Nz::CompoundCollider2D::New(std::move(colliders));
		collider->SetCollisionId(0);

		theCollider->AddComponent<Ndk::NodeComponent>().SetPosition(0.f, 0.f);
		theCollider->AddComponent<Ndk::CollisionComponent2D>(collider);
		auto& theColliderPhys = theCollider->AddComponent<Ndk::PhysicsComponent2D>();
		theColliderPhys.SetMass(0.f);
		theColliderPhys.SetFriction(1.f);


		/*Nz::MaterialRef burgerMat = Nz::Material::New("Translucent2D");
		burgerMat->SetDiffuseMap("../resources/burger.png");
		auto& sampler = burgerMat->GetDiffuseSampler();
		sampler.SetFilterMode(Nz::SamplerFilter_Nearest);

		Nz::SpriteRef burgerSprite = Nz::Sprite::New();
		burgerSprite->SetMaterial(burgerMat);
		burgerSprite->SetSize(burgerSprite->GetSize() / 2.f);
		Nz::Vector2f burgerSize = burgerSprite->GetSize();

		burgerSprite->SetOrigin(Nz::Vector2f(burgerSize.x / 2.f, burgerSize.y - 3.f));

		auto burgerBox = Nz::BoxCollider2D::New(Nz::Rectf(0.f, -burgerSize.y, burgerSize.x, burgerSize.y - 3.f));
		burgerBox->SetCollisionId(1);

		Ndk::EntityHandle burger = m_world.CreateEntity();
		burger->AddComponent<Ndk::GraphicsComponent>().Attach(burgerSprite);
		burger->AddComponent<Ndk::NodeComponent>().SetPosition(300.f, 100.f);
		burger->AddComponent<Ndk::CollisionComponent2D>(burgerBox);
		auto& burgerPhys = burger->AddComponent<Ndk::PhysicsComponent2D>();
		burgerPhys.SetMass(300);
		burgerPhys.SetFriction(10.f);
		burgerPhys.SetMomentOfInertia(std::numeric_limits<float>::infinity());*/
	}

	void LocalMatch::Update(float elapsedTime)
	{
		m_world.Update(elapsedTime);

		Ndk::PhysicsSystem2D::DebugDrawOptions options;
		options.polygonCallback = [](const Nz::Vector2f* vertices, std::size_t vertexCount, float radius, Nz::Color outline, Nz::Color fillColor, void* userData)
		{
			for (std::size_t i = 0; i < vertexCount - 1; ++i)
				Nz::DebugDrawer::DrawLine(vertices[i], vertices[i + 1]);

			Nz::DebugDrawer::DrawLine(vertices[vertexCount - 1], vertices[0]);
		};

		m_world.GetSystem<Ndk::PhysicsSystem2D>().DebugDraw(options);
	}

	const Ndk::EntityHandle& LocalMatch::CreateEntity(Nz::UInt32 serverId, const Nz::Vector2f& createPosition)
	{
		Nz::MaterialRef burgerMat = Nz::Material::New("Translucent2D");
		burgerMat->SetDiffuseMap("../resources/burger.png");
		auto& sampler = burgerMat->GetDiffuseSampler();
		sampler.SetFilterMode(Nz::SamplerFilter_Nearest);

		Nz::SpriteRef burgerSprite = Nz::Sprite::New();
		burgerSprite->SetMaterial(burgerMat);
		burgerSprite->SetSize(burgerSprite->GetSize() / 2.f);
		Nz::Vector2f burgerSize = burgerSprite->GetSize();

		burgerSprite->SetOrigin(Nz::Vector2f(burgerSize.x / 2.f, burgerSize.y - 3.f));

		auto burgerBox = Nz::BoxCollider2D::New(Nz::Rectf(-burgerSize.x / 2.f, -burgerSize.y, burgerSize.x, burgerSize.y - 3.f));
		burgerBox->SetCollisionId(1);

		const Ndk::EntityHandle& burger = m_world.CreateEntity();
		burger->AddComponent<Ndk::GraphicsComponent>().Attach(burgerSprite);
		burger->AddComponent<Ndk::NodeComponent>().SetPosition(createPosition);
		burger->AddComponent<Ndk::CollisionComponent2D>(burgerBox);
		auto& burgerPhys = burger->AddComponent<Ndk::PhysicsComponent2D>();
		burgerPhys.SetMass(300);
		burgerPhys.SetFriction(10.f);
		burgerPhys.SetMomentOfInertia(std::numeric_limits<float>::infinity());

		m_serverEntityIdToClient.emplace(serverId, burger);

		return burger;
	}

	void LocalMatch::DeleteEntity(Nz::UInt32 serverId)
	{
		auto it = m_serverEntityIdToClient.find(serverId);
		//assert(it != m_serverEntityIdToClient.end());
		if (it == m_serverEntityIdToClient.end())
			return;

		it->second->Kill();
		m_serverEntityIdToClient.erase(it);
	}

	void LocalMatch::MoveEntity(Nz::UInt32 serverId, const Nz::Vector2f& newPos)
	{
		auto it = m_serverEntityIdToClient.find(serverId);
		//assert(it != m_serverEntityIdToClient.end());
		if (it == m_serverEntityIdToClient.end())
			return;

		it->second->GetComponent<Ndk::PhysicsComponent2D>().SetPosition(newPos);
	}
}
