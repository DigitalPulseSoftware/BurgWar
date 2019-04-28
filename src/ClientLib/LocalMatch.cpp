// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/LocalMatch.hpp>
#include <ClientLib/ClientSession.hpp>
#include <ClientLib/InputController.hpp>
#include <ClientLib/LocalCommandStore.hpp>
#include <ClientLib/Components/LocalMatchComponent.hpp>
#include <ClientLib/Scripting/ClientGamemode.hpp>
#include <ClientLib/Scripting/ClientScriptingLibrary.hpp>
#include <ClientLib/Systems/SoundSystem.hpp>
#include <CoreLib/BurgApp.hpp>
#include <CoreLib/Components/AnimationComponent.hpp>
#include <CoreLib/Components/CooldownComponent.hpp>
#include <CoreLib/Components/InputComponent.hpp>
#include <CoreLib/Components/PlayerMovementComponent.hpp>
#include <CoreLib/Components/ScriptComponent.hpp>
#include <CoreLib/Systems/AnimationSystem.hpp>
#include <CoreLib/Systems/PlayerMovementSystem.hpp>
#include <CoreLib/Systems/TickCallbackSystem.hpp>
#include <Nazara/Graphics/ColorBackground.hpp>
#include <Nazara/Graphics/TileMap.hpp>
#include <Nazara/Graphics/TextSprite.hpp>
#include <Nazara/Renderer/DebugDrawer.hpp>
#include <Nazara/Platform/Keyboard.hpp>
#include <Nazara/Utility/SimpleTextDrawer.hpp>
#include <NDK/Components.hpp>
#include <NDK/Systems.hpp>
#include <cassert>
#include <iostream>

namespace bw
{
	LocalMatch::LocalMatch(BurgApp& burgApp, Nz::RenderTarget* renderTarget, ClientSession& session, const Packets::MatchData& matchData, std::shared_ptr<InputController> inputController) :
	SharedMatch(burgApp, matchData.tickDuration),
	m_inputController(std::move(inputController)),
	m_gamemodePath(matchData.gamemodePath),
	m_currentServerTick(matchData.currentTick),
	m_averageTickError(20),
	m_application(burgApp),
	m_session(session),
	m_errorCorrectionTimer(0.f),
	m_playerEntitiesTimer(0.f),
	m_playerInputTimer(0.f)
	{
		assert(renderTarget);

		m_world.AddSystem<AnimationSystem>(burgApp);
		m_world.AddSystem<PlayerMovementSystem>();
		m_world.AddSystem<SoundSystem>();
		m_world.AddSystem<TickCallbackSystem>();

		Ndk::RenderSystem& renderSystem = m_world.GetSystem<Ndk::RenderSystem>();
		renderSystem.SetGlobalUp(Nz::Vector3f::Down());
		renderSystem.SetDefaultBackground(Nz::ColorBackground::New(matchData.layers.front().backgroundColor));

		Ndk::PhysicsSystem2D& physics = m_world.GetSystem<Ndk::PhysicsSystem2D>();
		physics.SetGravity(Nz::Vector2f(0.f, 9.81f * 128.f));
		physics.SetStepSize(GetTickDuration());

		m_world.ForEachSystem([](Ndk::BaseSystem& system)
		{
			system.SetFixedUpdateRate(0.f);
			system.SetMaximumUpdateRate(0.f);
		});


		m_camera = m_world.CreateEntity();
		auto& cameraNode = m_camera->AddComponent<Ndk::NodeComponent>();
		cameraNode.SetPosition(-Nz::Vector2f(640.f, 360.f));

		Ndk::CameraComponent& viewer = m_camera->AddComponent<Ndk::CameraComponent>();
		viewer.SetTarget(renderTarget);
		viewer.SetProjectionType(Nz::ProjectionType_Orthogonal);

		Nz::Color trailColor(242, 255, 168);

		m_trailSpriteTest = Nz::Sprite::New();
		m_trailSpriteTest->SetMaterial(Nz::Material::New("Translucent2D"));
		m_trailSpriteTest->SetCornerColor(Nz::RectCorner_LeftBottom, trailColor * Nz::Color(128, 128, 128, 0));
		m_trailSpriteTest->SetCornerColor(Nz::RectCorner_LeftTop, trailColor * Nz::Color(128, 128, 128, 0));
		m_trailSpriteTest->SetCornerColor(Nz::RectCorner_RightTop, trailColor);
		m_trailSpriteTest->SetCornerColor(Nz::RectCorner_RightBottom, trailColor);
		m_trailSpriteTest->SetSize(64.f, 2.f);

		constexpr Nz::UInt8 playerCount = 1;

		m_inputPacket.inputs.resize(playerCount);
		for (auto& input : m_inputPacket.inputs)
			input.emplace();

		m_playerData.reserve(playerCount);
		assert(playerCount != 0xFF);
		for (Nz::UInt8 i = 0; i < playerCount; ++i)
			m_playerData.emplace_back(i);
	}

	void LocalMatch::ForEachEntity(std::function<void(const Ndk::EntityHandle& entity)> func)
	{
		for (const Ndk::EntityHandle& entity : m_world.GetEntities())
			func(entity);
	}

	void LocalMatch::LoadScripts(const std::shared_ptr<VirtualDirectory>& scriptDir)
	{
		m_scriptingContext = std::make_shared<ScriptingContext>(scriptDir);
		m_scriptingContext->LoadLibrary(std::make_shared<ClientScriptingLibrary>(*this));

		m_gamemode = std::make_shared<ClientGamemode>(*this, m_scriptingContext, m_gamemodePath);

		const std::string& gameResourceFolder = m_application.GetConfig().GetStringOption("Assets.ResourceFolder");

		m_entityStore.emplace(gameResourceFolder, m_scriptingContext);
		m_weaponStore.emplace(gameResourceFolder, m_scriptingContext);

		VirtualDirectory::Entry entry;

		if (scriptDir->GetEntry("entities", &entry))
			m_entityStore->Load("entities", std::get<VirtualDirectory::VirtualDirectoryEntry>(entry));

		if (scriptDir->GetEntry("weapons", &entry))
			m_weaponStore->Load("weapons", std::get<VirtualDirectory::VirtualDirectoryEntry>(entry));

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

		state["engine_AnimatePositionByOffsetSq"] = [&](const Ndk::EntityHandle& entity, const Nz::Vector2f& fromOffset, const Nz::Vector2f& toOffset, float duration, sol::object callbackObject)
		{
			m_animationManager.PushAnimation(duration, [=](float ratio)
			{
				if (!entity)
					return false;

				Nz::Vector2f offset = Nz::Lerp(fromOffset, toOffset, ratio * ratio); //< FIXME
				auto& nodeComponent = entity->GetComponent<Ndk::NodeComponent>();
				nodeComponent.SetInitialPosition(offset); //< FIXME

				return true;
			}, [this, callbackObject]()
			{
				sol::protected_function callback(m_scriptingContext->GetLuaState(), sol::ref_index(callbackObject.registry_index()));

				auto result = callback();
				if (!result.valid())
				{
					sol::error err = result;
					std::cerr << "engine_AnimatePositionByOffset callback failed: " << err.what() << std::endl;
				}
			});
			return 0;
		};

		state["engine_GetPlayerPosition"] = [&](Nz::UInt8 playerIndex)
		{
			if (playerIndex >= m_playerData.size())
				throw std::runtime_error("Invalid player index");

			auto& playerData = m_playerData[playerIndex];
			if (playerData.controlledEntity)
				return Nz::Vector2f(playerData.controlledEntity->GetComponent<Ndk::NodeComponent>().GetPosition());
			else
				return Nz::Vector2f::Zero();
		};

		state["engine_SetCameraPosition"] = [&](Nz::Vector2f position)
		{
			position.x = std::floor(position.x);
			position.y = std::floor(position.y);

			m_camera->GetComponent<Ndk::NodeComponent>().SetPosition(position);
		};

		m_gamemode->ExecuteCallback("OnInit");
	}

	void LocalMatch::Update(float elapsedTime)
	{
		if (m_scriptingContext)
			m_scriptingContext->Update();

		PrepareTickUpdate();

		SharedMatch::Update(elapsedTime);

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
		if (m_errorCorrectionTimer >= 1.f / ErrorCorrectionPerSecond)
		{
			// Compute how many loop we have to do (usually one)
			float loopCount = std::floor(ErrorCorrectionPerSecond * m_errorCorrectionTimer);
			m_errorCorrectionTimer -= loopCount / ErrorCorrectionPerSecond;

			// Compute correction factor for this loop count
			constexpr float positionCorrectionFactor = 0.3f;
			constexpr float rotationCorrectionFactor = 0.5f;

			float realPositionCorrectionFactor = Nz::IntegralPow(1.f - positionCorrectionFactor, static_cast<unsigned int>(loopCount));
			float realRotationCorrectionFactor = Nz::IntegralPow(1.f - rotationCorrectionFactor, static_cast<unsigned int>(loopCount));

			for (auto it = m_serverEntityIdToClient.begin(); it != m_serverEntityIdToClient.end(); ++it)
			{
				ServerEntity& serverEntity = it.value();
				if (!serverEntity.entity)
					continue;

				if (serverEntity.isPhysical)
				{
					auto& entityNode = serverEntity.entity->GetComponent<Ndk::NodeComponent>();
					auto& entityPhys = serverEntity.entity->GetComponent<Ndk::PhysicsComponent2D>();

					serverEntity.positionError *= realPositionCorrectionFactor;
					serverEntity.rotationError *= realRotationCorrectionFactor;

					// Avoid denormals
					for (std::size_t i = 0; i < 2; ++i)
					{
						if (Nz::NumberEquals(serverEntity.positionError[i], 0.f, 1.f))
							serverEntity.positionError[i] = 0.f;
					}

					if (serverEntity.rotationError == 0.f)
						serverEntity.rotationError = Nz::RadianAnglef::Zero();
				}
			}
		}

		for (auto it = m_serverEntityIdToClient.begin(); it != m_serverEntityIdToClient.end(); ++it)
		{
			ServerEntity& serverEntity = it.value();
			if (!serverEntity.entity)
				continue;

			if (serverEntity.isPhysical)
			{
				auto& entityNode = serverEntity.entity->GetComponent<Ndk::NodeComponent>();
				auto& entityPhys = serverEntity.entity->GetComponent<Ndk::PhysicsComponent2D>();

				// Apply new visual position/rotation
				entityNode.SetPosition(entityPhys.GetPosition() + serverEntity.positionError);
				entityNode.SetRotation(entityPhys.GetRotation() + serverEntity.rotationError);
			}
		}

		for (auto it = m_serverEntityIdToClient.begin(); it != m_serverEntityIdToClient.end(); ++it)
		{
			ServerEntity& serverEntity = it.value();
			if (!serverEntity.entity)
				continue;

			if (serverEntity.health)
			{
				auto& healthData = serverEntity.health.value();
				auto& entityNode = serverEntity.entity->GetComponent<Ndk::NodeComponent>();
				auto& entityGfx = serverEntity.entity->GetComponent<Ndk::GraphicsComponent>();

				const Nz::Boxf& aabb = entityGfx.GetAABB();

				auto& healthBarNode = healthData.healthBarEntity->GetComponent<Ndk::NodeComponent>();
				healthBarNode.SetPosition(aabb.GetCenter() - Nz::Vector3f(0.f, aabb.height / 2.f + 3.f, 0.f));
			}

			if (serverEntity.name)
			{
				auto& nameData = serverEntity.name.value();

				auto& entityNode = serverEntity.entity->GetComponent<Ndk::NodeComponent>();
				auto& entityGfx = serverEntity.entity->GetComponent<Ndk::GraphicsComponent>();

				const Nz::Boxf& aabb = entityGfx.GetAABB();

				auto& nameNode = nameData.nameEntity->GetComponent<Ndk::NodeComponent>();
				nameNode.SetPosition(aabb.GetCenter() - Nz::Vector3f(0.f, aabb.height / 2.f + 15, 0.f));
			}
		}

		constexpr float MaxInputSendInterval = 1.f / 60.f;
		constexpr float MinInputSendInterval = 1.f / 10.f;

		m_playerInputTimer += elapsedTime;
		m_timeSinceLastInputSending += elapsedTime;

		if (m_playerInputTimer >= MaxInputSendInterval)
		{
			m_playerInputTimer -= MaxInputSendInterval;
			if (SendInputs(m_timeSinceLastInputSending >= MinInputSendInterval))
				m_timeSinceLastInputSending = 0.f;
		}

		m_animationManager.Update(elapsedTime);
		if (m_gamemode)
			m_gamemode->ExecuteCallback("OnFrame");

		PrepareClientUpdate();

		m_world.Update(elapsedTime);
	}

	void LocalMatch::CreateHealthBar(ServerEntity& serverEntity, Nz::UInt16 currentHealth)
	{
		auto& healthData = serverEntity.health.emplace();
		healthData.currentHealth = currentHealth;

		auto& gfxComponent = serverEntity.entity->GetComponent<Ndk::GraphicsComponent>();

		const Nz::Boxf& aabb = gfxComponent.GetAABB();

		healthData.spriteWidth = std::max(aabb.width, aabb.height) * 0.85f;

		Nz::MaterialRef testMat = Nz::Material::New();
		testMat->EnableDepthBuffer(false);
		testMat->EnableFaceCulling(false);

		Nz::SpriteRef lostHealthBar = Nz::Sprite::New();
		lostHealthBar->SetMaterial(testMat);
		lostHealthBar->SetSize(healthData.spriteWidth, 10);
		lostHealthBar->SetColor(Nz::Color::Red);
		lostHealthBar->SetOrigin(Nz::Vector2f(healthData.spriteWidth / 2.f, lostHealthBar->GetSize().y));

		Nz::SpriteRef healthBar = Nz::Sprite::New();
		healthBar->SetMaterial(testMat);
		healthBar->SetSize(healthData.spriteWidth * healthData.currentHealth / serverEntity.maxHealth, 10);
		healthBar->SetColor(Nz::Color::Green);
		healthBar->SetOrigin(Nz::Vector2f(healthData.spriteWidth / 2.f, healthBar->GetSize().y));

		healthData.healthSprite = healthBar;

		healthData.healthBarEntity = m_world.CreateEntity();

		auto& healthBarGfx = healthData.healthBarEntity->AddComponent<Ndk::GraphicsComponent>();
		healthBarGfx.Attach(healthBar, 2);
		healthBarGfx.Attach(lostHealthBar, 1);

		healthData.healthBarEntity->AddComponent<Ndk::NodeComponent>();
	}

	void LocalMatch::CreateName(ServerEntity& serverEntity, const std::string& name)
	{
		auto& nameData = serverEntity.name.emplace();
		
		Nz::TextSpriteRef nameSprite = Nz::TextSprite::New();
		nameSprite->Update(Nz::SimpleTextDrawer::Draw(name, 24, Nz::TextStyle_Regular, Nz::Color::White, 2.f, Nz::Color::Black));

		Nz::Boxf textBox = nameSprite->GetBoundingVolume().obb.localBox;

		nameData.nameEntity = m_world.CreateEntity();
		nameData.nameEntity->AddComponent<Ndk::NodeComponent>();
	
		auto& gfxComponent = nameData.nameEntity->AddComponent<Ndk::GraphicsComponent>();
		gfxComponent.Attach(nameSprite, Nz::Matrix4f::Translate(Nz::Vector2f(-textBox.width / 2.f, -textBox.height)), 3);
	}

	void LocalMatch::DebugEntityId(ServerEntity& serverEntity)
	{
		auto& gfxComponent = serverEntity.entity->GetComponent<Ndk::GraphicsComponent>();
		auto& nodeComponent = serverEntity.entity->GetComponent<Ndk::NodeComponent>();

		const Nz::Boxf& aabb = gfxComponent.GetAABB();
		Nz::Vector3f offset = nodeComponent.GetPosition() - aabb.GetCenter();

		Nz::TextSpriteRef text = Nz::TextSprite::New(Nz::SimpleTextDrawer::Draw("S: " + std::to_string(serverEntity.serverEntityId) + ", C: " + std::to_string(serverEntity.entity->GetId()), 36));
		Nz::Boxf volume = text->GetBoundingVolume().obb.localBox;

		gfxComponent.Attach(text, Nz::Matrix4f::Translate(Nz::Vector3f(aabb.width / 2.f - volume.width / 2.f, aabb.height / 2 - 5 - volume.height / 2.f, 0.f)));
	}

	Nz::UInt16 LocalMatch::EstimateServerTick() const
	{
		return m_currentServerTick - m_averageTickError.GetAverageValue();
	}

	void LocalMatch::HandleTickPacket(TickPacketContent&& packet)
	{
		std::visit([this](auto&& packet)
		{
			HandleTickPacket(std::move(packet));
		}, std::move(packet));
	}

	void LocalMatch::HandleTickPacket(Packets::ControlEntity&& packet)
	{
		auto it = m_serverEntityIdToClient.find(packet.entityId);
		if (it == m_serverEntityIdToClient.end())
			return;

		const ServerEntity& serverEntity = it->second;

		if (m_playerData[packet.playerIndex].controlledEntity)
			m_playerData[packet.playerIndex].controlledEntity->RemoveComponent<Ndk::ListenerComponent>();

		m_playerData[packet.playerIndex].controlledEntity = serverEntity.entity;
		m_playerData[packet.playerIndex].controlledEntity->AddComponent<Ndk::ListenerComponent>();

		//m_camera->GetComponent<Ndk::NodeComponent>().SetParent(serverEntity.entity);
	}

	void LocalMatch::HandleTickPacket(Packets::CreateEntities&& packet)
	{
		static std::string entityPrefix = "entity_";
		static std::string weaponPrefix = "weapon_";

		const NetworkStringStore& networkStringStore = m_session.GetNetworkStringStore();

		for (auto&& entityData : packet.entities)
		{
			const std::string& entityClass = networkStringStore.GetString(entityData.entityClass);

			EntityProperties properties;
			for (const auto& property : entityData.properties)
			{
				const std::string& propertyName = networkStringStore.GetString(property.name);

				std::visit([&](auto&& value)
				{
					using T = std::decay_t<decltype(value)>;

					if constexpr (std::is_same_v<T, std::vector<bool>> ||
						std::is_same_v<T, std::vector<float>> ||
						std::is_same_v<T, std::vector<Nz::Int64>> ||
						std::is_same_v<T, std::vector<Nz::Vector2f>> ||
						std::is_same_v<T, std::vector<Nz::Vector2i64>> ||
						std::is_same_v<T, std::vector<std::string>>)
					{
						using StoredType = typename T::value_type;

						if (property.isArray)
						{
							EntityPropertyArray<StoredType> elements(value.size());
							for (std::size_t i = 0; i < value.size(); ++i)
								elements[i] = value[i];

							properties.emplace(propertyName, std::move(elements));
						}
						else
							properties.emplace(propertyName, value.front());
					}
					else
						static_assert(AlwaysFalse<T>::value, "non-exhaustive visitor");

				}, property.value);
			}

			/*const*/ ServerEntity* parent = nullptr;
			if (entityData.parentId)
			{
				auto it = m_serverEntityIdToClient.find(entityData.parentId.value());
				assert(it != m_serverEntityIdToClient.end());

				//parent = &it->second;
				parent = &it.value();
			}

			Ndk::EntityHandle entity;
			if (entityClass.compare(0, entityPrefix.size(), entityPrefix) == 0)
			{
				// Entity
				if (std::size_t entityIndex = m_entityStore->GetElementIndex(entityClass); entityIndex != ClientEntityStore::InvalidIndex)
				{
					entity = m_entityStore->InstantiateEntity(m_world, entityIndex, entityData.position, entityData.rotation, properties);
					if (!entity)
						continue;
				}
			}
			else if (entityClass.compare(0, weaponPrefix.size(), weaponPrefix) == 0)
			{
				// Weapon
				if (std::size_t weaponIndex = m_weaponStore->GetElementIndex(entityClass); weaponIndex != ClientEntityStore::InvalidIndex)
				{
					assert(parent);

					parent->weaponEntityId = entityData.id; //< TEMPORARY

					entity = m_weaponStore->InstantiateWeapon(m_world, weaponIndex, properties, parent->entity);
					if (!entity)
						continue;

					entity->GetComponent<Ndk::NodeComponent>().SetPosition(entityData.position);
				}
			}
			else
			{
				// Unknown
				std::cerr << "Failed to decode entity type: " << entityClass << std::endl;
				continue;
			}

			if (entity)
			{
				ServerEntity serverEntity;
				serverEntity.entity = entity;
				serverEntity.isPhysical = entityData.physicsProperties.has_value();
				serverEntity.maxHealth = (entityData.health.has_value()) ? entityData.health->maxHealth : 0;
				serverEntity.serverEntityId = entityData.id;

				entity->AddComponent<LocalMatchComponent>(shared_from_this());
				//entity->AddComponent<Ndk::DebugComponent>(Ndk::DebugDraw::Collider2D | Ndk::DebugDraw::GraphicsAABB | Ndk::DebugDraw::GraphicsOBB);
				//DebugEntityId(serverEntity);

				if (entityData.health && entityData.health->currentHealth != entityData.health->maxHealth)
					CreateHealthBar(serverEntity, entityData.health->currentHealth);

				if (entityData.name)
					CreateName(serverEntity, entityData.name.value());

				m_serverEntityIdToClient.emplace(entityData.id, std::move(serverEntity));
			}
		}
	}

	void LocalMatch::HandleTickPacket(Packets::DeleteEntities&& packet)
	{
		for (auto&& entityData : packet.entities)
		{
			auto it = m_serverEntityIdToClient.find(entityData.id);
			//assert(it != m_serverEntityIdToClient.end());
			if (it == m_serverEntityIdToClient.end())
				return;

			m_serverEntityIdToClient.erase(it);
		}
	}

	void LocalMatch::HandleTickPacket(Packets::EntitiesAnimation&& packet)
	{
		for (auto&& entityData : packet.entities)
		{
			auto it = m_serverEntityIdToClient.find(entityData.entityId);
			if (it == m_serverEntityIdToClient.end())
				continue;

			ServerEntity& serverEntity = it.value();
			if (!serverEntity.entity)
				continue;

			auto& animComponent = serverEntity.entity->GetComponent<AnimationComponent>();
			animComponent.Play(entityData.animId, m_application.GetAppTime());
		}
	}

	void LocalMatch::HandleTickPacket(Packets::EntitiesInputs&& packet)
	{
		for (auto&& entityData : packet.entities)
		{
			auto it = m_serverEntityIdToClient.find(entityData.id);
			if (it == m_serverEntityIdToClient.end())
				continue;

			ServerEntity& serverEntity = it.value();
			if (!serverEntity.entity)
				continue;

			serverEntity.entity->GetComponent<InputComponent>().UpdateInputs(entityData.inputs);

			// TEMPORARY
			if (serverEntity.weaponEntityId != 0xFFFFFFFF)
			{
				auto weaponIt = m_serverEntityIdToClient.find(serverEntity.weaponEntityId);
				if (weaponIt == m_serverEntityIdToClient.end())
					return;

				ServerEntity& weaponEntity = weaponIt.value();
				if (!weaponEntity.entity)
					return;

				if (entityData.inputs.isAttacking)
				{
					auto& weaponCooldown = weaponEntity.entity->GetComponent<CooldownComponent>();
					if (weaponCooldown.Trigger(GetCurrentTime()))
					{
						auto& weaponScript = weaponEntity.entity->GetComponent<ScriptComponent>();
						weaponScript.ExecuteCallback("OnAttack", weaponScript.GetTable());
					}
				}
			}
		}
	}

	void LocalMatch::HandleTickPacket(Packets::HealthUpdate&& packet)
	{
		for (auto&& entityData : packet.entities)
		{
			auto it = m_serverEntityIdToClient.find(entityData.id);
			if (it == m_serverEntityIdToClient.end())
				continue;

			ServerEntity& serverEntity = it.value();
			if (!serverEntity.entity)
				continue;

			if (serverEntity.health)
			{
				HealthData& healthData = serverEntity.health.value();
				healthData.currentHealth = entityData.currentHealth;
				healthData.healthSprite->SetSize(healthData.spriteWidth * healthData.currentHealth / serverEntity.maxHealth, 10);
			}
			else
				CreateHealthBar(serverEntity, entityData.currentHealth);
		}
	}

	void LocalMatch::HandleTickPacket(Packets::MatchState&& packet)
	{
		if (Nz::Keyboard::IsKeyPressed(Nz::Keyboard::A))
			return;

		for (auto&& entityData : packet.entities)
		{
			auto it = m_serverEntityIdToClient.find(entityData.id);
			//assert(it != m_serverEntityIdToClient.end());
			if (it == m_serverEntityIdToClient.end())
				return;

			ServerEntity& serverEntity = it.value();
			if (!serverEntity.entity)
				return;

			if (serverEntity.isPhysical)
			{
				assert(entityData.physicsProperties);

				auto& physComponent = serverEntity.entity->GetComponent<Ndk::PhysicsComponent2D>();

				serverEntity.positionError += physComponent.GetPosition() - entityData.position;
				serverEntity.rotationError += physComponent.GetRotation() - entityData.rotation;

				if (serverEntity.entity->HasComponent<PlayerMovementComponent>())
				{
					auto& playerMovementComponent = serverEntity.entity->GetComponent<PlayerMovementComponent>();

					if (playerMovementComponent.UpdateFacingRightState(entityData.playerMovement->isFacingRight))
					{
						auto& entityNode = serverEntity.entity->GetComponent<Ndk::NodeComponent>();
						entityNode.Scale(-1.f, 1.f);
					}
				}

				physComponent.SetPosition(entityData.position);
				physComponent.SetRotation(entityData.rotation);

				physComponent.SetAngularVelocity(entityData.physicsProperties->angularVelocity);
				physComponent.SetVelocity(entityData.physicsProperties->linearVelocity);
			}
			else
			{
				auto& nodeComponent = serverEntity.entity->GetComponent<Ndk::NodeComponent>();
				nodeComponent.SetPosition(entityData.position);
				nodeComponent.SetRotation(entityData.rotation);
			}
		}
	}

	void LocalMatch::HandleTickError(Nz::Int32 tickError)
	{
		m_averageTickError.InsertValue(m_averageTickError.GetAverageValue() + tickError);
	}

	void LocalMatch::OnTick()
	{
		Nz::UInt16 estimatedServerTick = EstimateServerTick() /* + 3 for network jitter */;

		auto it = m_tickedPackets.begin();
		while (it != m_tickedPackets.end() && estimatedServerTick >= it->tick)
		{
			HandleTickPacket(std::move(it->content));
			++it;
		}
		m_tickedPackets.erase(m_tickedPackets.begin(), it);

		m_world.Update(GetTickDuration());

		if (m_gamemode)
			m_gamemode->ExecuteCallback("OnTick");

		m_currentServerTick++;
	}

	void LocalMatch::PrepareClientUpdate()
	{
		m_world.ForEachSystem([](Ndk::BaseSystem& system)
		{
			system.Enable(false);
		});

		m_world.GetSystem<Ndk::DebugSystem>().Enable(true);
		m_world.GetSystem<Ndk::ListenerSystem>().Enable(true);
		m_world.GetSystem<Ndk::ParticleSystem>().Enable(true);
		m_world.GetSystem<Ndk::RenderSystem>().Enable(true);
		m_world.GetSystem<AnimationSystem>().Enable(true);
		m_world.GetSystem<SoundSystem>().Enable(true);
	}

	void LocalMatch::PrepareTickUpdate()
	{
		m_world.ForEachSystem([](Ndk::BaseSystem& system)
		{
			system.Enable(false);
		});

		m_world.GetSystem<Ndk::LifetimeSystem>().Enable(true);
		m_world.GetSystem<Ndk::PhysicsSystem2D>().Enable(true);
		m_world.GetSystem<Ndk::PhysicsSystem3D>().Enable(true);
		m_world.GetSystem<Ndk::VelocitySystem>().Enable(true);
		m_world.GetSystem<PlayerMovementSystem>().Enable(true);
		m_world.GetSystem<TickCallbackSystem>().Enable(true);
	}

	void LocalMatch::PushTickPacket(Nz::UInt16 tick, TickPacketContent&& packet)
	{
		TickPacket newPacket;
		newPacket.tick = tick;
		newPacket.content = std::move(packet);

		auto it = std::upper_bound(m_tickedPackets.begin(), m_tickedPackets.end(), newPacket, [](const TickPacket& a, const TickPacket& b)
		{
			return a.tick < b.tick;
		});

		m_tickedPackets.emplace(it, std::move(newPacket));
	}

	bool LocalMatch::SendInputs(bool force)
	{
		assert(m_playerData.size() == m_inputPacket.inputs.size());

		m_inputPacket.estimatedServerTick = EstimateServerTick();

		bool hasInputData = false;
		for (std::size_t i = 0; i < m_playerData.size(); ++i)
		{
			auto& controllerData = m_playerData[i];
			InputData input = m_inputController->Poll(*this, controllerData.playerIndex, controllerData.controlledEntity);

			if (controllerData.lastInputData != input)
			{
				hasInputData = true;
				controllerData.lastInputData = input;
				m_inputPacket.inputs[i] = input;
			}
			else
				m_inputPacket.inputs[i].reset();
		}

		if (hasInputData || force)
		{
			m_session.SendPacket(m_inputPacket);
			return true;
		}
		else
			return false;
	}
}
