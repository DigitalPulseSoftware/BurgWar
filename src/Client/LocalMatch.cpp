// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/LocalMatch.hpp>
#include <Client/ClientApp.hpp>
#include <Client/ClientSession.hpp>
#include <Shared/Components/AnimationComponent.hpp>
#include <Shared/Components/PlayerMovementComponent.hpp>
#include <Shared/Components/ScriptComponent.hpp>
#include <Shared/Systems/AnimationSystem.hpp>
#include <Shared/Systems/PlayerMovementSystem.hpp>
#include <Shared/Systems/TickCallbackSystem.hpp>
#include <Nazara/Graphics/ColorBackground.hpp>
#include <Nazara/Graphics/TileMap.hpp>
#include <Nazara/Renderer/DebugDrawer.hpp>
#include <Nazara/Platform/Keyboard.hpp>
#include <NDK/Components.hpp>
#include <NDK/LuaAPI.hpp>
#include <NDK/Systems.hpp>
#include <cassert>
#include <iostream>

namespace bw
{
	LocalMatch::LocalMatch(ClientApp& burgApp, ClientSession& session, const Packets::MatchData& matchData) :
	m_application(burgApp),
	m_session(session),
	m_errorCorrectionTimer(0.f),
	m_playerEntitiesTimer(0.f),
	m_playerInputTimer(0.f)
	{
		m_world.AddSystem<AnimationSystem>(burgApp);
		m_world.AddSystem<PlayerMovementSystem>();
		m_world.AddSystem<TickCallbackSystem>();

		Ndk::RenderSystem& renderSystem = m_world.GetSystem<Ndk::RenderSystem>();
		renderSystem.SetGlobalUp(Nz::Vector3f::Down());
		renderSystem.SetDefaultBackground(Nz::ColorBackground::New(matchData.backgroundColor));

		Ndk::PhysicsSystem2D& physics = m_world.GetSystem<Ndk::PhysicsSystem2D>();
		physics.SetGravity(Nz::Vector2f(0.f, 9.81f * 128.f));
		/*physics.SetMaximumUpdateRate(20.f);
		physics.SetMaxStepCount(3);
		physics.SetStepSize(1.f / 40.f);*/

		Ndk::EntityHandle camera = m_world.CreateEntity();
		camera->AddComponent<Ndk::NodeComponent>().SetPosition(Nz::Vector2f(0.f, 0.f));

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

		constexpr Nz::UInt8 playerCount = 1;

		m_inputPacket.inputs.resize(playerCount);
		for (auto& input : m_inputPacket.inputs)
			input.emplace();

		m_inputControllers.reserve(playerCount);
		assert(playerCount != 0xFF);
		for (Nz::UInt8 i = 0; i < playerCount; ++i)
			m_inputControllers.emplace_back(m_application, i);

		/*auto& networkStringStore = m_session.GetNetworkStringStore();
		m_entityStore.ForEachElement([&](const ScriptedEntity& entity)
		{
			if (entity.isNetworked)
			{
				if (networkStringStore.GetStringIndex(entity.name) == NetworkStringStore::InvalidIndex)
					std::cout << "[Client] Entity " << entity.name << " is marked as networked but is not part of the network string store" << std::endl;
			}
		});*/

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

	void LocalMatch::LoadScripts(const std::shared_ptr<VirtualDirectory>& scriptDir)
	{
		m_scriptingContext = std::make_shared<ClientScriptingContext>(*this, scriptDir);

		m_entityStore.emplace(nullptr, m_scriptingContext);
		m_weaponStore.emplace(nullptr, m_scriptingContext);

		VirtualDirectory::Entry entry;

		if (scriptDir->GetEntry("entities", &entry))
			m_entityStore->Load("entities", std::get<VirtualDirectory::DirectoryEntry>(entry));

		if (scriptDir->GetEntry("weapons", &entry))
			m_weaponStore->Load("weapons", std::get<VirtualDirectory::DirectoryEntry>(entry));

		sol::state& state = m_scriptingContext->GetLuaState();
		state["engine_AnimateRotation"] = [&](const Ndk::EntityHandle& entity, float fromAngle, float toAngle, float duration, sol::object callbackObject)
		{
			m_animationManager.PushAnimation(duration, [=](float ratio)
			{
				if (!entity)
					return false;

				float newAngle = Nz::Lerp(fromAngle, toAngle, ratio);
				auto& nodeComponent = entity->GetComponent<Ndk::NodeComponent>();
				nodeComponent.SetRotation(Nz::DegreeAnglef(newAngle));

				return true;
			}, [this, callbackObject]()
			{
				sol::protected_function callback(m_scriptingContext->GetLuaState(), sol::ref_index(callbackObject.registry_index()));

				auto result = callback();
				if (!result.valid())
				{
					sol::error err = result;
					std::cerr << "engine_AnimateRotation callback failed: " << err.what() << std::endl;
				}
			});
			return 0;
		};
	}

	void LocalMatch::Update(float elapsedTime)
	{
		if (m_scriptingContext)
			m_scriptingContext->Update();

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
				if (!serverEntity.entity)
					continue;

				if (serverEntity.isPhysical)
				{
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
		}

		constexpr float MaxInputSendInterval = 1.f / 60.f;

		m_playerInputTimer += elapsedTime;
		if (m_playerInputTimer >= MaxInputSendInterval)
		{
			m_playerInputTimer -= MaxInputSendInterval;
			SendInputs();
		}

		m_animationManager.Update(elapsedTime);
	}

	Ndk::EntityHandle LocalMatch::CreateEntity(Nz::UInt32 serverId, const std::string& entityClassName, const Nz::Vector2f& createPosition, bool hasPlayerMovement, bool isPhysical, std::optional<Nz::UInt32> parentId, Nz::UInt16 currentHealth, Nz::UInt16 maxHealth)
	{
		static std::string entityPrefix = "entity_";
		static std::string weaponPrefix = "weapon_";

		const ServerEntity* parent = nullptr;
		if (parentId)
		{
			auto it = m_serverEntityIdToClient.find(parentId.value());
			assert(it != m_serverEntityIdToClient.end());

			parent = &it->second;
		}

		Ndk::EntityHandle entity;
		if (entityClassName.compare(0, entityPrefix.size(), entityPrefix) == 0)
		{
			// Entity
			if (std::size_t entityIndex = m_entityStore->GetElementIndex(entityClassName); entityIndex != ClientEntityStore::InvalidIndex)
			{
				entity = m_entityStore->InstantiateEntity(m_world, entityIndex);
				if (!entity)
					return Ndk::EntityHandle::InvalidHandle;

				entity->GetComponent<Ndk::NodeComponent>().SetPosition(createPosition);
			}
		}
		else if (entityClassName.compare(0, weaponPrefix.size(), weaponPrefix) == 0)
		{
			// Weapon
			if (std::size_t weaponIndex = m_weaponStore->GetElementIndex(entityClassName); weaponIndex != ClientEntityStore::InvalidIndex)
			{
				assert(parent);

				entity = m_weaponStore->InstantiateWeapon(m_world, weaponIndex, parent->entity);
				if (!entity)
					return Ndk::EntityHandle::InvalidHandle;

				entity->GetComponent<Ndk::NodeComponent>().SetPosition(createPosition);
			}
		}
		else
		{
			// Unknown
			std::cerr << "Failed to decode entity type: " << entityClassName << std::endl;
			return Ndk::EntityHandle::InvalidHandle;
		}

		if (entity)
		{
			ServerEntity serverEntity;
			serverEntity.entity = entity;
			serverEntity.isPhysical = isPhysical;

			if (maxHealth != 0)
			{
				auto& healthData = serverEntity.health.emplace();
				healthData.currentHealth = currentHealth;
				healthData.maxHealth = maxHealth;

				auto& gfxComponent = entity->GetComponent<Ndk::GraphicsComponent>();
				auto& nodeComponent = entity->GetComponent<Ndk::NodeComponent>();

				Nz::Boxf aabb = gfxComponent.GetAABB();

				Nz::MaterialRef testMat = Nz::Material::New();
				testMat->EnableDepthBuffer(false);
				testMat->EnableFaceCulling(false);

				Nz::SpriteRef lostHealthBar = Nz::Sprite::New();
				lostHealthBar->SetMaterial(testMat);
				lostHealthBar->SetSize(aabb.width, 10);
				lostHealthBar->SetColor(Nz::Color::Red);

				Nz::SpriteRef healthBar = Nz::Sprite::New();
				healthBar->SetMaterial(testMat);
				healthBar->SetSize(aabb.width * healthData.currentHealth / healthData.maxHealth, 10);
				healthBar->SetColor(Nz::Color::Green);

				Nz::Vector3f position = aabb.GetPosition() - Nz::Vector3f(0.f, healthBar->GetSize().y + 3.f, 0.f);
				position -= nodeComponent.GetPosition();

				gfxComponent.Attach(healthBar, Nz::Matrix4f::Translate(position), 2);
				gfxComponent.Attach(lostHealthBar, Nz::Matrix4f::Translate(position), 1);

				healthData.spriteWidth = aabb.width;
				healthData.healthSprite = healthBar;
			}

			m_serverEntityIdToClient.emplace(serverId, std::move(serverEntity));
		}

		return entity;
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
		if (!serverEntity.entity)
			return;

		if (serverEntity.isPhysical)
		{
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
			physComponent.SetRotation(newRot);
			physComponent.SetVelocity(newLinearVel);
		}
		else
		{
			auto& nodeComponent = serverEntity.entity->GetComponent<Ndk::NodeComponent>();
			nodeComponent.SetPosition(newPos);
			nodeComponent.SetRotation(newRot);
		}
	}

	void LocalMatch::PlayAnimation(Nz::UInt32 serverId, Nz::UInt8 animId)
	{
		auto it = m_serverEntityIdToClient.find(serverId);
		if (it == m_serverEntityIdToClient.end())
			return;

		ServerEntity& serverEntity = it.value();
		if (!serverEntity.entity)
			return;

		auto& animComponent = serverEntity.entity->GetComponent<AnimationComponent>();
		animComponent.Play(animId, m_application.GetAppTime());
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

	void LocalMatch::UpdateEntityHealth(Nz::UInt32 serverId, Nz::UInt16 newHealth)
	{
		auto it = m_serverEntityIdToClient.find(serverId);
		if (it == m_serverEntityIdToClient.end())
			return;

		ServerEntity& serverEntity = it.value();
		if (!serverEntity.entity)
			return;

		assert(serverEntity.health);
		HealthData& healthData = serverEntity.health.value();
		healthData.currentHealth = newHealth;
		healthData.healthSprite->SetSize(healthData.spriteWidth * healthData.currentHealth / healthData.maxHealth, 10);
	}
}
