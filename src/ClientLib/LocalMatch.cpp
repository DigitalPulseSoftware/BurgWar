// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/LocalMatch.hpp>
#include <ClientLib/ClientSession.hpp>
#include <ClientLib/LocalCommandStore.hpp>
#include <ClientLib/Scripting/ClientGamemode.hpp>
#include <ClientLib/Scripting/ClientScriptingLibrary.hpp>
#include <CoreLib/BurgApp.hpp>
#include <CoreLib/Components/AnimationComponent.hpp>
#include <CoreLib/Components/InputComponent.hpp>
#include <CoreLib/Components/PlayerMovementComponent.hpp>
#include <CoreLib/Components/ScriptComponent.hpp>
#include <CoreLib/Systems/AnimationSystem.hpp>
#include <CoreLib/Systems/PlayerMovementSystem.hpp>
#include <CoreLib/Systems/TickCallbackSystem.hpp>
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
	LocalMatch::LocalMatch(BurgApp& burgApp, Nz::RenderTarget* renderTarget, ClientSession& session, const Packets::MatchData& matchData) :
	SharedMatch(burgApp),
	m_gamemodePath(matchData.gamemodePath),
	m_application(burgApp),
	m_session(session),
	m_errorCorrectionTimer(0.f),
	m_playerEntitiesTimer(0.f),
	m_playerInputTimer(0.f)
	{
		assert(renderTarget);

		m_world.AddSystem<AnimationSystem>(burgApp);
		m_world.AddSystem<PlayerMovementSystem>();
		m_world.AddSystem<TickCallbackSystem>();

		Ndk::RenderSystem& renderSystem = m_world.GetSystem<Ndk::RenderSystem>();
		renderSystem.SetGlobalUp(Nz::Vector3f::Down());
		renderSystem.SetDefaultBackground(Nz::ColorBackground::New(matchData.layers.front().backgroundColor));

		Ndk::PhysicsSystem2D& physics = m_world.GetSystem<Ndk::PhysicsSystem2D>();
		physics.SetGravity(Nz::Vector2f(0.f, 9.81f * 128.f));
		physics.SetMaximumUpdateRate(30.f);
		/*physics.SetMaximumUpdateRate(20.f);
		physics.SetMaxStepCount(3);
		physics.SetStepSize(1.f / 40.f);*/

		m_camera = m_world.CreateEntity();
		auto& cameraNode = m_camera->AddComponent<Ndk::NodeComponent>();
		cameraNode.SetPosition(-Nz::Vector2f(640.f, 360.f));

		Ndk::CameraComponent& viewer = m_camera->AddComponent<Ndk::CameraComponent>();
		viewer.SetTarget(renderTarget);
		viewer.SetProjectionType(Nz::ProjectionType_Orthogonal);

		auto& layerData = matchData.layers.front();

		constexpr Nz::UInt8 playerCount = 1;

		m_inputPacket.inputs.resize(playerCount);
		for (auto& input : m_inputPacket.inputs)
			input.emplace();

		m_inputControllers.reserve(playerCount);
		assert(playerCount != 0xFF);
		for (Nz::UInt8 i = 0; i < playerCount; ++i)
			m_inputControllers.emplace_back(i);
	}

	void LocalMatch::LoadScripts(const std::shared_ptr<VirtualDirectory>& scriptDir)
	{
		m_scriptingContext = std::make_shared<ClientScriptingContext>(scriptDir);
		m_scriptingContext->LoadLibrary(std::make_shared<ClientScriptingLibrary>(*this));

		m_gamemode = std::make_shared<ClientGamemode>(*this, m_scriptingContext, m_gamemodePath);

		m_entityStore.emplace(m_scriptingContext);
		m_weaponStore.emplace(m_scriptingContext);

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

		state["engine_GetPlayerPosition"] = [&]()
		{
			if (!m_playerControlledEntity)
				return Nz::Vector2f::Zero();

			return Nz::Vector2f(m_playerControlledEntity->GetComponent<Ndk::NodeComponent>().GetPosition());
		};

		state["engine_SetCameraPosition"] = [&](const Nz::Vector2f& position)
		{
			m_camera->GetComponent<Ndk::NodeComponent>().SetPosition(position);
		};

		m_gamemode->ExecuteCallback("OnInit");
	}

	void LocalMatch::Update(float elapsedTime)
	{
		SharedMatch::Update(elapsedTime);

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

				// Apply new position/rotation
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

		m_animationManager.Update(elapsedTime);
		if (m_gamemode)
			m_gamemode->ExecuteCallback("OnTick");
	}

	void LocalMatch::ControlEntity(Nz::UInt32 serverId)
	{
		auto it = m_serverEntityIdToClient.find(serverId);
		if (it == m_serverEntityIdToClient.end())
			return;

		const ServerEntity& serverEntity = it->second;
		m_playerControlledEntity = serverEntity.entity;
		//m_camera->GetComponent<Ndk::NodeComponent>().SetParent(serverEntity.entity);
	}

	Ndk::EntityHandle LocalMatch::CreateEntity(Nz::UInt32 serverId, const std::string& entityClassName, const Nz::Vector2f& createPosition, bool hasPlayerMovement, bool hasInputs, bool isPhysical, std::optional<Nz::UInt32> parentId, Nz::UInt16 currentHealth, Nz::UInt16 maxHealth, const EntityProperties& properties)
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
				entity = m_entityStore->InstantiateEntity(m_world, entityIndex, createPosition, 0, properties);
				if (!entity)
					return Ndk::EntityHandle::InvalidHandle;
			}
		}
		else if (entityClassName.compare(0, weaponPrefix.size(), weaponPrefix) == 0)
		{
			// Weapon
			if (std::size_t weaponIndex = m_weaponStore->GetElementIndex(entityClassName); weaponIndex != ClientEntityStore::InvalidIndex)
			{
				assert(parent);

				entity = m_weaponStore->InstantiateWeapon(m_world, weaponIndex, properties, parent->entity);
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

	void LocalMatch::MoveEntity(Nz::UInt32 serverId, const Nz::Vector2f& newPos, const Nz::Vector2f& newLinearVel, Nz::RadianAnglef newRot, Nz::RadianAnglef newAngularVel, bool isFacingRight)
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

	void LocalMatch::UpdateEntityInput(Nz::UInt32 serverId, const InputData& inputs)
	{
		auto it = m_serverEntityIdToClient.find(serverId);
		if (it == m_serverEntityIdToClient.end())
			return;

		ServerEntity& serverEntity = it.value();
		if (!serverEntity.entity)
			return;

		serverEntity.entity->GetComponent<InputComponent>().UpdateInputs(inputs);
	}
}
