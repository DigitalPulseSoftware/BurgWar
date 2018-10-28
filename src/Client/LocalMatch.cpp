// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/LocalMatch.hpp>
#include <Client/BurgApp.hpp>
#include <Client/ClientSession.hpp>
#include <Shared/Components/PlayerMovementComponent.hpp>
#include <Shared/Systems/PlayerMovementSystem.hpp>
#include <Nazara/Graphics/ColorBackground.hpp>
#include <Nazara/Graphics/TileMap.hpp>
#include <Nazara/Renderer/DebugDrawer.hpp>
#include <Nazara/Platform/Keyboard.hpp>
#include <NDK/Components.hpp>
#include <NDK/Systems.hpp>
#include <cassert>
#include <iostream>

namespace bw
{
	LocalMatch::LocalMatch(BurgApp& burgApp, ClientSession& session, const Packets::MatchData& matchData) :
	m_application(burgApp),
	m_session(session),
	m_hasInputChanged(false),
	m_errorCorrectionTimer(0.f),
	m_playerEntitiesTimer(0.f),
	m_playerInputTimer(0.f)
	{
		m_world.AddSystem<PlayerMovementSystem>();

		Ndk::RenderSystem& renderSystem = m_world.GetSystem<Ndk::RenderSystem>();
		renderSystem.SetGlobalUp(Nz::Vector3f::Down());
		renderSystem.SetDefaultBackground(Nz::ColorBackground::New(matchData.backgroundColor));

		Ndk::PhysicsSystem2D& physics = m_world.GetSystem<Ndk::PhysicsSystem2D>();
		physics.SetGravity(Nz::Vector2f(0.f, 9.81f * 128.f));

		Ndk::EntityHandle camera = m_world.CreateEntity();
		camera->AddComponent<Ndk::NodeComponent>().SetPosition(Nz::Vector2f(320.f, 0.f));

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

		auto& layerData = matchData.layers.front();

		Nz::TileMapRef tileMap = Nz::TileMap::New(Nz::Vector2ui(layerData.width, layerData.height), Nz::Vector2f(matchData.tileSize, matchData.tileSize));
		tileMap->SetMaterial(0, dirtMat);

		Ndk::EntityHandle theCollider = m_world.CreateEntity();
		auto& worldGfx = theCollider->AddComponent<Ndk::GraphicsComponent>();

		for (Nz::UInt16 y = 0; y < layerData.height; ++y)
		{
			for (Nz::UInt16 x = 0; x < layerData.width; ++x)
			{
				switch (layerData.tiles[y * layerData.width + x])
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
		for (std::size_t y = 0; y < layerData.height; ++y)
		{
			for (std::size_t x = 0; x < layerData.width; ++x)
			{
				if (layerData.tiles[y * layerData.width + x] != 0)
				{
					std::size_t startX = x++;

					while (x < layerData.width && layerData.tiles[y * layerData.width + x] != 0) ++x;

					std::cout << "[client] " << Nz::Rectf(startX * tileMap->GetTileSize().x, y * tileMap->GetTileSize().y, (x - startX) * tileMap->GetTileSize().x, tileMap->GetTileSize().y) << std::endl;
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

		m_inputPacket.isJumping = false;
		m_inputPacket.isMovingLeft = false;
		m_inputPacket.isMovingRight = false;

		Nz::EventHandler& eventHandler = m_application.GetMainWindow().GetEventHandler();
		m_onKeyPressedSlot.Connect(eventHandler.OnKeyPressed, [this](const Nz::EventHandler* /*eventHandler*/, const Nz::WindowEvent::KeyEvent& event)
		{
			if (event.repeated)
				return;

			switch (event.code)
			{
				case Nz::Keyboard::Space:
					m_inputPacket.isJumping = true;
					m_hasInputChanged = true;
					break;

				case Nz::Keyboard::Q:
					m_inputPacket.isMovingLeft = true;
					m_hasInputChanged = true;
					break;

				case Nz::Keyboard::D:
					m_inputPacket.isMovingRight = true;
					m_hasInputChanged = true;
					break;

				default:
					break;
			}
		});

		m_onKeyReleasedSlot.Connect(eventHandler.OnKeyReleased, [this](const Nz::EventHandler* /*eventHandler*/, const Nz::WindowEvent::KeyEvent& event)
		{
			switch (event.code)
			{
				case Nz::Keyboard::Space:
					m_inputPacket.isJumping = false;
					m_hasInputChanged = true;
					break;

				case Nz::Keyboard::Q:
					m_inputPacket.isMovingLeft = false;
					m_hasInputChanged = true;
					break;

				case Nz::Keyboard::D:
					m_inputPacket.isMovingRight = false;
					m_hasInputChanged = true;
					break;

				default:
					break;
			}
		});

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

		/*Ndk::PhysicsSystem2D::DebugDrawOptions options;
		options.polygonCallback = [](const Nz::Vector2f* vertices, std::size_t vertexCount, float radius, Nz::Color outline, Nz::Color fillColor, void* userData)
		{
			for (std::size_t i = 0; i < vertexCount - 1; ++i)
				Nz::DebugDrawer::DrawLine(vertices[i], vertices[i + 1]);

			Nz::DebugDrawer::DrawLine(vertices[vertexCount - 1], vertices[0]);
		};

		m_world.GetSystem<Ndk::PhysicsSystem2D>().DebugDraw(options);*/

		constexpr float ErrorCorrectionPerSecond = 60;

		m_errorCorrectionTimer += elapsedTime;
		while (m_errorCorrectionTimer >= 1.f / ErrorCorrectionPerSecond)
		{
			m_errorCorrectionTimer -= 1.f / ErrorCorrectionPerSecond;

			for (auto it = m_serverEntityIdToClient.begin(); it != m_serverEntityIdToClient.end(); ++it)
			{
				ServerEntity& serverEntity = it.value();
				auto& entityNode = serverEntity.entity->GetComponent<Ndk::NodeComponent>();
				auto& entityPhys = serverEntity.entity->GetComponent<Ndk::PhysicsComponent2D>();

				serverEntity.positionError = Nz::Lerp(serverEntity.positionError, Nz::Vector2f::Zero(), 0.5f);

				// Avoid denormals
				for (std::size_t i = 0; i < 2; ++i)
				{
					if (Nz::NumberEquals(serverEntity.positionError[i], 0.f, 1.f))
						serverEntity.positionError[i] = 0.f;
				}

				serverEntity.rotationError = Nz::Lerp(serverEntity.rotationError, Nz::RadianAnglef::Zero(), 0.5f);
				if (serverEntity.rotationError == 0.f)
					serverEntity.rotationError = Nz::RadianAnglef::Zero();

				entityNode.SetPosition(entityPhys.GetPosition() + serverEntity.positionError);
				entityNode.SetRotation(entityPhys.GetRotation() + serverEntity.rotationError);
			}
		}

		constexpr float MaxInputSendInterval = 1.f / 60.f;

		m_playerInputTimer += elapsedTime;
		if (m_playerInputTimer >= MaxInputSendInterval)
		{
			m_playerInputTimer -= MaxInputSendInterval;

			if (m_hasInputChanged)
			{
				m_session.SendPacket(m_inputPacket);
				m_hasInputChanged = false;
			}
		}
	}

	const Ndk::EntityHandle& LocalMatch::CreateEntity(Nz::UInt32 serverId, const Nz::Vector2f& createPosition, bool hasPlayerMovement)
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
		auto& burgerGfx = burger->AddComponent<Ndk::NodeComponent>();
		burgerGfx.SetPosition(createPosition);

		burger->AddComponent<Ndk::CollisionComponent2D>(burgerBox);
		auto& burgerPhys = burger->AddComponent<Ndk::PhysicsComponent2D>();
		burgerPhys.SetMass(300);
		burgerPhys.SetFriction(10.f);
		burgerPhys.SetMomentOfInertia(std::numeric_limits<float>::infinity());
		burgerPhys.EnableNodeSynchronization(false);

		if (hasPlayerMovement)
			burger->AddComponent<PlayerMovementComponent>();

		ServerEntity serverEntity;
		serverEntity.entity = burger;

		m_serverEntityIdToClient.emplace(serverId, std::move(serverEntity));

		return burger;
	}

	void LocalMatch::DeleteEntity(Nz::UInt32 serverId)
	{
		auto it = m_serverEntityIdToClient.find(serverId);
		//assert(it != m_serverEntityIdToClient.end());
		if (it == m_serverEntityIdToClient.end())
			return;

		m_serverEntityIdToClient.erase(it);
	}

	void LocalMatch::MoveEntity(Nz::UInt32 serverId, const Nz::Vector2f& newPos, const Nz::Vector2f& newLinearVel, Nz::RadianAnglef newRot, Nz::RadianAnglef newAngularVel, bool isAirControlling, bool isFacingRight)
	{
		if (Nz::Keyboard::IsKeyPressed(Nz::Keyboard::A))
			return;

		auto it = m_serverEntityIdToClient.find(serverId);
		//assert(it != m_serverEntityIdToClient.end());
		if (it == m_serverEntityIdToClient.end())
			return;

		ServerEntity& serverEntity = it.value();
		auto& physComponent = serverEntity.entity->GetComponent<Ndk::PhysicsComponent2D>();

		serverEntity.positionError += physComponent.GetPosition() - newPos;
		serverEntity.rotationError += physComponent.GetRotation() - newRot;

		if (serverEntity.entity->HasComponent<PlayerMovementComponent>())
		{
			auto& playerMovementComponent = serverEntity.entity->GetComponent<PlayerMovementComponent>();
			playerMovementComponent.UpdateAirControlState(isAirControlling);

			if (playerMovementComponent.UpdateFacingRightState(isFacingRight))
			{
				auto& entityNode = serverEntity.entity->GetComponent<Ndk::NodeComponent>();
				entityNode.Scale(-1.f, 1.f);
			}
		}

		physComponent.SetAngularVelocity(newAngularVel);
		physComponent.SetPosition(newPos);
		physComponent.SetVelocity(newLinearVel);
	}
}
