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
	m_errorCorrectionTimer(0.f),
	m_playerEntitiesTimer(0.f),
	m_playerInputTimer(0.f),
	m_entityStore(false, m_luaInstance)
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

		constexpr Nz::UInt8 playerCount = 2;

		m_inputPacket.inputs.resize(playerCount);
		for (auto& input : m_inputPacket.inputs)
			input.emplace();

		m_inputControllers.reserve(playerCount);
		assert(playerCount != 0xFF);
		for (Nz::UInt8 i = 0; i < playerCount; ++i)
			m_inputControllers.emplace_back(i);

		m_luaInstance.LoadLibraries();
		m_entityStore.Load("../../scripts/entities");

		auto& networkStringStore = m_session.GetNetworkStringStore();
		m_entityStore.ForEachEntity([&](const EntityStore::Entity& entity)
		{
			if (entity.isNetworked)
			{
				if (networkStringStore.GetStringIndex(entity.name) == NetworkStringStore::InvalidIndex)
					std::cout << "[Client] Entity " << entity.name << " is marked as networked but is not part of the network string store" << std::endl;
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
			SendInputs();
		}
	}

	const Ndk::EntityHandle& LocalMatch::CreateEntity(Nz::UInt32 serverId, const std::string& entityClassName, const Nz::Vector2f& createPosition, bool hasPlayerMovement)
	{
		std::size_t entityIndex = m_entityStore.GetEntityIndex(entityClassName);
		if (entityIndex == EntityStore::InvalidIndex)
		{
			std::cerr << "Entity class \"" << entityClassName << "\" is not registered" << std::endl;
			return Ndk::EntityHandle::InvalidHandle;
		}

		auto& entityClass = m_entityStore.GetEntity(entityIndex);

		std::string spritePath;
		bool canRotate;
		float mass;
		float scale;
		unsigned int collisionId;
		try
		{
			m_luaInstance.PushReference(entityClass.tableRef);
			Nz::CallOnExit popOnExit([&] { m_luaInstance.Pop(); });

			canRotate = m_luaInstance.CheckField<bool>("RotationEnabled");
			collisionId = m_luaInstance.CheckField<unsigned int>("CollisionType");
			mass = m_luaInstance.CheckField<float>("Mass");
			scale = m_luaInstance.CheckField<float>("Scale");
			spritePath = m_luaInstance.CheckField<std::string>("Sprite");
		}
		catch (const std::exception& e)
		{
			std::cerr << "Failed to get entity class \"" << entityClassName << "\" informations: " << e.what() << std::endl;
			return Ndk::EntityHandle::InvalidHandle;
		}

		Nz::MaterialRef burgerMat = Nz::Material::New("Translucent2D");
		burgerMat->SetDiffuseMap(spritePath);
		auto& sampler = burgerMat->GetDiffuseSampler();
		sampler.SetFilterMode(Nz::SamplerFilter_Bilinear);

		Nz::SpriteRef burgerSprite = Nz::Sprite::New();
		burgerSprite->SetMaterial(burgerMat);
		burgerSprite->SetSize(burgerSprite->GetSize() * scale);
		Nz::Vector2f burgerSize = burgerSprite->GetSize();

		// Warning what's following is ugly
		Nz::Rectf colliderBox;
		if (entityClassName == "burger")
		{
			colliderBox = Nz::Rectf(-burgerSize.x / 2.f, -burgerSize.y, burgerSize.x, burgerSize.y - 3.f);
			burgerSprite->SetOrigin(Nz::Vector2f(burgerSize.x / 2.f, burgerSize.y - ((entityClassName == "burger") ? 3.f : 0.f)));
		}
		else
		{
			colliderBox = Nz::Rectf(-burgerSize.x / 2.f, -burgerSize.y / 2.f, burgerSize.x, burgerSize.y);
			burgerSprite->SetOrigin(Nz::Vector2f(burgerSize.x / 2.f, burgerSize.y / 2.f));
		}

		auto burgerBox = Nz::BoxCollider2D::New(colliderBox);
		burgerBox->SetCollisionId(collisionId);

		const Ndk::EntityHandle& burger = m_world.CreateEntity();
		burger->AddComponent<Ndk::GraphicsComponent>().Attach(burgerSprite);
		auto& burgerGfx = burger->AddComponent<Ndk::NodeComponent>();
		burgerGfx.SetPosition(createPosition);

		burger->AddComponent<Ndk::CollisionComponent2D>(burgerBox);
		auto& burgerPhys = burger->AddComponent<Ndk::PhysicsComponent2D>();
		burgerPhys.SetMass(mass);
		burgerPhys.SetFriction(10.f);
		if (!canRotate)
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

	void LocalMatch::SendInputs()
	{
		assert(m_inputControllers.size() == m_inputPacket.inputs.size());

		bool hasInputData = false;
		for (std::size_t i = 0; i < m_inputControllers.size(); ++i)
		{
			auto& controllerData = m_inputControllers[i];
			InputData input = controllerData.controller.Poll();

			if (controllerData.lastInputData != input)
			{
				hasInputData = true;
				controllerData.lastInputData = input;
				m_inputPacket.inputs[i] = input;
			}
			else
				m_inputPacket.inputs[i].reset();
		}

		if (hasInputData)
			m_session.SendPacket(m_inputPacket);
	}
}
