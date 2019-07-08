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
#include <CoreLib/Components/WeaponComponent.hpp>
#include <CoreLib/Systems/AnimationSystem.hpp>
#include <CoreLib/Systems/PlayerMovementSystem.hpp>
#include <CoreLib/Systems/TickCallbackSystem.hpp>
#include <CoreLib/Systems/WeaponSystem.hpp>
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
#include <fstream>

namespace bw
{
	LocalMatch::LocalMatch(BurgApp& burgApp, Nz::RenderWindow* window, Ndk::Canvas* canvas, ClientSession& session, const Packets::MatchData& matchData, std::shared_ptr<InputController> inputController) :
	SharedMatch(burgApp, matchData.tickDuration),
	m_inputController(std::move(inputController)),
	m_gamemodePath(matchData.gamemodePath),
	m_averageTickError(20),
	m_window(window),
	m_application(burgApp),
	m_chatBox(window, canvas),
	m_session(session),
	m_world(burgApp, *this),
	m_errorCorrectionTimer(0.f),
	m_playerEntitiesTimer(0.f),
	m_playerInputTimer(0.f)
	{
		assert(window);

		m_averageTickError.InsertValue(-static_cast<Nz::Int32>(matchData.currentTick));

		Ndk::World& world = m_world.GetWorld();
		world.AddSystem<SoundSystem>();

		Ndk::RenderSystem& renderSystem = world.GetSystem<Ndk::RenderSystem>();
		renderSystem.SetGlobalUp(Nz::Vector3f::Down());
		renderSystem.SetDefaultBackground(Nz::ColorBackground::New(matchData.layers.front().backgroundColor));

		m_camera = world.CreateEntity();
		auto& cameraNode = m_camera->AddComponent<Ndk::NodeComponent>();
		cameraNode.SetPosition(-Nz::Vector2f(640.f, 360.f));

		Ndk::CameraComponent& viewer = m_camera->AddComponent<Ndk::CameraComponent>();
		viewer.SetTarget(window);
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

		m_prediction.emplace(*this);

		m_chatBox.OnChatMessage.Connect([this](const std::string& message)
		{
			Packets::PlayerChat chatPacket;
			chatPacket.playerIndex = 0;
			chatPacket.message = message;

			m_session.SendPacket(chatPacket);
		});

		m_inputController->OnSwitchWeapon.Connect([this](InputController* /*emitter*/, Nz::UInt8 localPlayerIndex, bool direction)
		{
			assert(localPlayerIndex < m_playerData.size());
			auto& playerData = m_playerData[localPlayerIndex];

			if (direction)
			{
				if (++playerData.selectedWeapon > playerData.weapons.size())
					playerData.selectedWeapon = 0;
			}
			else
			{
				if (playerData.selectedWeapon-- == 0)
					playerData.selectedWeapon = playerData.weapons.size();
			}

			Packets::PlayerSelectWeapon selectPacket;
			selectPacket.playerIndex = localPlayerIndex;
			selectPacket.newWeaponIndex = (playerData.selectedWeapon < playerData.weapons.size()) ? playerData.selectedWeapon : selectPacket.NoWeapon;

			m_session.SendPacket(selectPacket);
		});

		onUnhandledKeyPressed.Connect(canvas->OnUnhandledKeyPressed, [this](const Nz::EventHandler*, const Nz::WindowEvent::KeyEvent& event)
		{
			switch (event.code)
			{
				case Nz::Keyboard::Return:
					m_chatBox.Open(!m_chatBox.IsOpen());
					break;

				default:
					break;
			}
		});

		if (m_application.GetConfig().GetBoolOption("Debug.ShowServerGhosts"))
		{
			m_debug.emplace();
			if (m_debug->socket.Create(Nz::NetProtocol_IPv4))
			{
				m_debug->socket.EnableBlocking(false);

				Nz::IpAddress localhost = Nz::IpAddress::LoopbackIpV4;
				for (std::size_t i = 0; i < 4; ++i)
				{
					localhost.SetPort(static_cast<Nz::UInt16>(42000 + i));

					if (m_debug->socket.Bind(localhost) == Nz::SocketState_Bound)
						break;
				}

				if (m_debug->socket.GetState() == Nz::SocketState_Bound)
				{
					std::cout << "Debug socket bound to port " << m_debug->socket.GetBoundPort() << std::endl;
				}
				else
				{
					std::cerr << "Failed to bind debug socket";
					m_debug.reset();
				}
			}
			else
			{
				std::cerr << "Failed to create debug socket";
				m_debug.reset();
			}
		}
	}

	LocalMatch::~LocalMatch()
	{
		// Clear timer manager before scripting context gets deleted
		GetTimerManager().Clear();
	}

	void LocalMatch::ForEachEntity(std::function<void(const Ndk::EntityHandle& entity)> func)
	{
		Ndk::World& world = m_world.GetWorld();
		for (const Ndk::EntityHandle& entity : world.GetEntities())
			func(entity);
	}

	SharedWorld& LocalMatch::GetWorld()
	{
		return m_world;
	}

	void LocalMatch::LoadScripts()
	{
		assert(m_scriptingDirectory);
		LoadScripts(m_scriptingDirectory);
	}

	void LocalMatch::LoadScripts(const std::shared_ptr<VirtualDirectory>& scriptDir)
	{
		m_scriptingDirectory = scriptDir;

		if (!m_scriptingContext)
		{
			std::shared_ptr<ClientScriptingLibrary> scriptingLibrary = std::make_shared<ClientScriptingLibrary>(*this);

			m_scriptingContext = std::make_shared<ScriptingContext>(m_scriptingDirectory);
			m_scriptingContext->LoadLibrary(scriptingLibrary);

			if (!m_console)
				m_console.emplace(m_window, m_canvas, scriptingLibrary, m_scriptingDirectory);
		}
		else
			m_scriptingContext->ReloadLibraries();

		const std::string& gameResourceFolder = m_application.GetConfig().GetStringOption("Assets.ResourceFolder");

		m_entityStore.emplace(gameResourceFolder, m_scriptingContext);
		m_weaponStore.emplace(gameResourceFolder, m_scriptingContext);

		VirtualDirectory::Entry entry;

		if (m_scriptingDirectory->GetEntry("entities", &entry))
			m_entityStore->Load("entities", std::get<VirtualDirectory::VirtualDirectoryEntry>(entry));

		if (m_scriptingDirectory->GetEntry("weapons", &entry))
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

		state["engine_GetPlayerPosition"] = [&](sol::this_state lua, Nz::UInt8 playerIndex) -> sol::object
		{
			if (playerIndex >= m_playerData.size())
				throw std::runtime_error("Invalid player index");

			auto& playerData = m_playerData[playerIndex];
			if (playerData.controlledEntity)
				return sol::make_object(lua, Nz::Vector2f(playerData.controlledEntity->GetComponent<Ndk::NodeComponent>().GetPosition()));
			else
				return sol::nil;
		};

		state["engine_GetCameraViewport"] = [&]()
		{
			return m_camera->GetComponent<Ndk::CameraComponent>().GetTarget()->GetSize();
		};

		state["engine_SetCameraPosition"] = [&](Nz::Vector2f position)
		{
			position.x = std::floor(position.x);
			position.y = std::floor(position.y);

			m_camera->GetComponent<Ndk::NodeComponent>().SetPosition(position);
		};

		ForEachEntity([this](const Ndk::EntityHandle& entity)
		{
			if (entity->HasComponent<ScriptComponent>())
			{
				// Warning: ugly (FIXME)
				m_entityStore->UpdateEntityElement(entity);
				m_weaponStore->UpdateEntityElement(entity);
			}
		});

		if (!m_gamemode)
		{
			m_gamemode = std::make_shared<ClientGamemode>(*this, m_scriptingContext, m_gamemodePath);
			m_gamemode->ExecuteCallback("OnInit");
		}
		else
			m_gamemode->Reload();
	}

	void LocalMatch::Update(float elapsedTime)
	{
		if (m_scriptingContext)
			m_scriptingContext->Update();

		ProcessInputs(elapsedTime);

		PrepareTickUpdate();

		SharedMatch::Update(elapsedTime);

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

		if (m_debug)
		{
			Nz::NetPacket debugPacket;
			while (m_debug->socket.ReceivePacket(&debugPacket, nullptr))
			{
				switch (debugPacket.GetNetCode())
				{
					case 1: //< StatePacket
					{
						Nz::UInt32 entityCount;
						debugPacket >> entityCount;

						for (Nz::UInt32 i = 0; i < entityCount; ++i)
						{
							CompressedUnsigned<Nz::UInt32> entityId;
							debugPacket >> entityId;

							bool isPhysical;
							Nz::Vector2f linearVelocity;
							Nz::RadianAnglef angularVelocity;
							Nz::Vector2f position;
							Nz::RadianAnglef rotation;

							debugPacket >> isPhysical;

							if (isPhysical)
								debugPacket >> linearVelocity >> angularVelocity;

							debugPacket >> position >> rotation;

							if (auto it = m_serverEntityIdToClient.find(entityId); it != m_serverEntityIdToClient.end())
							{
								ServerEntity& serverEntity = it.value();
								if (serverEntity.serverGhost)
								{
									if (isPhysical && serverEntity.serverGhost->HasComponent<Ndk::PhysicsComponent2D>())
									{
										auto& ghostPhysics = serverEntity.serverGhost->GetComponent<Ndk::PhysicsComponent2D>();
										ghostPhysics.SetPosition(position);
										ghostPhysics.SetRotation(rotation);
										ghostPhysics.SetAngularVelocity(angularVelocity);
										ghostPhysics.SetVelocity(linearVelocity);
									}
									else
									{
										auto& ghostNode = serverEntity.serverGhost->GetComponent<Ndk::NodeComponent>();
										ghostNode.SetPosition(position);
										ghostNode.SetRotation(rotation);
									}
								}
							}
						}

						break;
					}

					default:
						break;
				}
			}
		}

		m_animationManager.Update(elapsedTime);
		if (m_gamemode)
			m_gamemode->ExecuteCallback("OnFrame");

		PrepareClientUpdate();

		Ndk::World& world = m_world.GetWorld();
		world.Update(elapsedTime);

		/*Ndk::PhysicsSystem2D::DebugDrawOptions options;
		options.polygonCallback = [](const Nz::Vector2f* vertices, std::size_t vertexCount, float radius, Nz::Color outline, Nz::Color fillColor, void* userData)
		{
			for (std::size_t i = 0; i < vertexCount - 1; ++i)
				Nz::DebugDrawer::DrawLine(vertices[i], vertices[i + 1]);

			Nz::DebugDrawer::DrawLine(vertices[vertexCount - 1], vertices[0]);
		};

		m_world.GetSystem<Ndk::PhysicsSystem2D>().DebugDraw(options);*/
	}

	void LocalMatch::CreateGhostEntity(ServerEntity& serverEntity)
	{
		Ndk::World& world = m_world.GetWorld();
		serverEntity.serverGhost = world.CreateEntity();
		serverEntity.serverGhost->AddComponent(serverEntity.entity->GetComponent<Ndk::NodeComponent>().Clone());

		if (serverEntity.entity->HasComponent<Ndk::PhysicsComponent2D>())
		{
			auto& ghostPhysics = static_cast<Ndk::PhysicsComponent2D&>(serverEntity.serverGhost->AddComponent(serverEntity.entity->GetComponent<Ndk::PhysicsComponent2D>().Clone()));
			ghostPhysics.SetMass(0.f); //< Turns into kinematic
		}

		if (serverEntity.entity->HasComponent<Ndk::GraphicsComponent>())
		{
			auto& originalGraphics = serverEntity.entity->GetComponent<Ndk::GraphicsComponent>();
			auto& ghostGraphics = serverEntity.serverGhost->AddComponent<Ndk::GraphicsComponent>();

			ghostGraphics.Clear();

			originalGraphics.ForEachRenderable([&](const Nz::InstancedRenderableRef& renderable, const Nz::Matrix4f& localMatrix, int /*renderOrder*/)
			{
				if (std::unique_ptr<Nz::InstancedRenderable> clonedRenderable = renderable->Clone())
				{
					std::size_t materialCount = clonedRenderable->GetMaterialCount();
					for (std::size_t i = 0; i < materialCount; ++i)
					{
						Nz::MaterialRef ghostMaterial = Nz::Material::New(*clonedRenderable->GetMaterial(i));
						ghostMaterial->Configure("Translucent2D");
						ghostMaterial->SetDiffuseColor(Nz::Color(255, 255, 255, 160));
						ghostMaterial->SetDiffuseMap(ghostMaterial->GetDiffuseMap());

						clonedRenderable->SetMaterial(i, ghostMaterial);
					}

					ghostGraphics.Attach(clonedRenderable.release(), localMatrix, -1);
				}
			});
		}
	}

	void LocalMatch::CreateHealthBar(ServerEntity& serverEntity, Nz::UInt16 currentHealth)
	{
		Ndk::World& world = m_world.GetWorld();
		
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

		healthData.healthBarEntity = world.CreateEntity();

		auto& healthBarGfx = healthData.healthBarEntity->AddComponent<Ndk::GraphicsComponent>();
		healthBarGfx.Attach(healthBar, 2);
		healthBarGfx.Attach(lostHealthBar, 1);

		healthData.healthBarEntity->AddComponent<Ndk::NodeComponent>();
	}

	void LocalMatch::CreateName(ServerEntity& serverEntity, const std::string& name)
	{
		Ndk::World& world = m_world.GetWorld();
		
		auto& nameData = serverEntity.name.emplace();
		
		Nz::TextSpriteRef nameSprite = Nz::TextSprite::New();
		nameSprite->Update(Nz::SimpleTextDrawer::Draw(name, 24, Nz::TextStyle_Regular, Nz::Color::White, 2.f, Nz::Color::Black));

		Nz::Boxf textBox = nameSprite->GetBoundingVolume().obb.localBox;

		nameData.nameEntity = world.CreateEntity();
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

	Nz::UInt64 LocalMatch::EstimateServerTick() const
	{
		return GetCurrentTick() - m_averageTickError.GetAverageValue();
	}

	void LocalMatch::HandleChatMessage(Packets::ChatMessage&& packet)
	{
		m_chatBox.PrintMessage(packet.content);
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

		ServerEntity& serverEntity = it.value();
		serverEntity.isLocalPlayerControlled = true;

		if (m_playerData[packet.playerIndex].controlledEntity)
			m_playerData[packet.playerIndex].controlledEntity->RemoveComponent<Ndk::ListenerComponent>();

		m_playerData[packet.playerIndex].controlledEntity = serverEntity.entity;
		m_playerData[packet.playerIndex].controlledEntity->AddComponent<Ndk::ListenerComponent>();
		m_playerData[packet.playerIndex].controlledEntityServerId = packet.entityId;
	}

	void LocalMatch::HandleTickPacket(Packets::CreateEntities&& packet)
	{
		static std::string entityPrefix = "entity_";
		static std::string weaponPrefix = "weapon_";

		Ndk::World& world = m_world.GetWorld();

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
					entity = m_entityStore->InstantiateEntity(world, entityIndex, entityData.position, entityData.rotation, properties);
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

					entity = m_weaponStore->InstantiateWeapon(world, weaponIndex, properties, parent->entity);
					if (!entity)
						continue;

					entity->GetComponent<Ndk::NodeComponent>().SetPosition(entityData.position);
					entity->Disable(); //< Disable weapon entities by default
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

				entity->AddComponent<LocalMatchComponent>(*this);
				//entity->AddComponent<Ndk::DebugComponent>(Ndk::DebugDraw::Collider2D | Ndk::DebugDraw::GraphicsAABB | Ndk::DebugDraw::GraphicsOBB);
				//DebugEntityId(serverEntity);

				if (m_debug)
					CreateGhostEntity(serverEntity);

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
			m_prediction->DeleteEntity(entityData.id);

			auto it = m_serverEntityIdToClient.find(entityData.id);
			//assert(it != m_serverEntityIdToClient.end());
			if (it == m_serverEntityIdToClient.end())
				continue;

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

			if (!serverEntity.isLocalPlayerControlled)
				serverEntity.entity->GetComponent<InputComponent>().UpdateInputs(entityData.inputs);
		}
	}

	void LocalMatch::HandleTickPacket(Packets::EntityWeapon&& packet)
	{
		auto it = m_serverEntityIdToClient.find(packet.entityId);
		if (it == m_serverEntityIdToClient.end())
			return;

		ServerEntity& serverEntity = it.value();
		if (!serverEntity.entity)
			return;

		if (serverEntity.weaponEntityId != ServerEntity::NoWeapon)
		{
			auto weaponIt = m_serverEntityIdToClient.find(serverEntity.weaponEntityId);
			if (weaponIt != m_serverEntityIdToClient.end())
			{
				ServerEntity& serverEntity = weaponIt.value();
				if (serverEntity.entity)
				{
					auto& entityWeapon = serverEntity.entity->GetComponent<WeaponComponent>();
					entityWeapon.SetActive(false);
				}

				serverEntity.entity->Disable();
			}
		}

		serverEntity.weaponEntityId = packet.weaponEntityId;

		if (serverEntity.weaponEntityId != ServerEntity::NoWeapon)
		{
			auto weaponIt = m_serverEntityIdToClient.find(serverEntity.weaponEntityId);
			if (weaponIt != m_serverEntityIdToClient.end())
			{
				ServerEntity& serverEntity = weaponIt.value();
				if (serverEntity.entity)
				{
					auto& entityWeapon = serverEntity.entity->GetComponent<WeaponComponent>();
					entityWeapon.SetActive(true);
				}

				serverEntity.entity->Enable();
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

		Ndk::World& world = m_world.GetWorld();

#ifdef DEBUG_PREDICTION
		static std::ofstream debugFile("prediction.txt", std::ios::trunc);
		debugFile << "---------------------------------------------------\n";
		debugFile << "Received match state for tick #" << packet.stateTick << " (current estimation: " << EstimateServerTick() << ")\n";
#endif

		// Remove treated inputs
		auto firstClientInput = std::find_if(m_predictedInputs.begin(), m_predictedInputs.end(), [stateTick = packet.stateTick](const PredictedInput& input)
		{
			return input.serverTick > stateTick;
		});
		m_predictedInputs.erase(m_predictedInputs.begin(), firstClientInput);

		auto& physicsSystem = world.GetSystem<Ndk::PhysicsSystem2D>();

		for (auto&& entityData : packet.entities)
		{
			auto it = m_serverEntityIdToClient.find(entityData.id);
			//assert(it != m_serverEntityIdToClient.end());
			if (it == m_serverEntityIdToClient.end())
				return;

			ServerEntity& serverEntity = it.value();
			if (!serverEntity.entity)
				return;

			bool isPredicted = false;

			for (std::size_t playerIndex = 0; playerIndex < m_playerData.size(); ++playerIndex)
			{
				auto& controllerData = m_playerData[playerIndex];
				if (controllerData.controlledEntityServerId == entityData.id && controllerData.controlledEntity)
				{
					isPredicted = true;

#ifdef DEBUG_PREDICTION
					if (serverEntity.entity->HasComponent<InputComponent>())
					{
						debugFile << "Burger position: " << entityData.position.y << "\n";
						debugFile << "Burger velocity: " << entityData.physicsProperties->linearVelocity.y << "\n";
					}
#endif

					m_prediction->RegisterForPrediction(controllerData.controlledEntity, [](const Ndk::EntityHandle& entity)
					{
						entity->AddComponent<InputComponent>();
						entity->AddComponent<PlayerMovementComponent>();
					}, [&](const Ndk::EntityHandle& /*sourceEntity*/, const Ndk::EntityHandle& targetEntity)
					{
						auto& entityPhys = targetEntity->GetComponent<Ndk::PhysicsComponent2D>();
						entityPhys.SetPosition(entityData.position);
						entityPhys.SetRotation(entityData.rotation);
						entityPhys.SetAngularVelocity(entityData.physicsProperties->angularVelocity);
						entityPhys.SetVelocity(entityData.physicsProperties->linearVelocity);
					});

					break;
				}
			}

			if (!isPredicted)
			{
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

					physComponent.SetAngularVelocity(entityData.physicsProperties->angularVelocity);
					physComponent.SetPosition(entityData.position);
					physComponent.SetRotation(entityData.rotation);
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

		// Player entities and surrounding entities should be predicted
		for (std::size_t playerIndex = 0; playerIndex < m_playerData.size(); ++playerIndex)
		{
			auto& controllerData = m_playerData[playerIndex];
			if (controllerData.controlledEntity)
			{
				if (controllerData.controlledEntity->HasComponent<Ndk::CollisionComponent2D>())
				{
					Nz::Vector2f position;
					if (controllerData.controlledEntity->HasComponent<Ndk::PhysicsComponent2D>())
						position = controllerData.controlledEntity->GetComponent<Ndk::PhysicsComponent2D>().GetPosition();
					else
						position = Nz::Vector2f(controllerData.controlledEntity->GetComponent<Ndk::NodeComponent>().GetPosition());

					physicsSystem.RegionQuery(Nz::Rectf(position.x - 500, position.y - 500, 1000.f, 1000.f), 0, 0xFFFFFFFF, 0xFFFFFFFF, [&](const Ndk::EntityHandle& entity)
					{
						// Prevent registering player entities a second time (as it would resync their physics which we don't want)
						if (!m_prediction->IsRegistered(entity->GetId()))
						{
							m_prediction->RegisterForPrediction(entity, [](const Ndk::EntityHandle& entity)
							{
								if (entity->HasComponent<Ndk::PhysicsComponent2D>())
									entity->GetComponent<Ndk::PhysicsComponent2D>().SetMass(0.f, false); //< Treat every dynamic object as kinematic
							});
						}
					});
				}
			}
		}

		m_prediction->DeleteUnregisteredEntities();

		for (const PredictedInput& input : m_predictedInputs)
		{
			for (std::size_t i = 0; i < m_playerData.size(); ++i)
			{
				auto& controllerData = m_playerData[i];
				if (controllerData.controlledEntity)
				{
					const Ndk::EntityHandle& reconciliationEntity = m_prediction->GetEntity(controllerData.controlledEntity->GetId());
					assert(reconciliationEntity);

					InputComponent& entityInputs = reconciliationEntity->GetComponent<InputComponent>();
					const auto& playerInputData = input.inputs[i];
					entityInputs.UpdateInputs(playerInputData.input);

#ifdef DEBUG_PREDICTION
					debugFile << "---- prediction (input tick: #" << input.serverTick << ", jumping: " << std::boolalpha << playerInputData.input.isJumping << ")\n";
#endif

					if (playerInputData.movement)
					{
						auto& playerMovement = reconciliationEntity->GetComponent<PlayerMovementComponent>();
						auto& playerPhysics = reconciliationEntity->GetComponent<Ndk::PhysicsComponent2D>();
						playerMovement.UpdateGroundState(playerInputData.movement->isOnGround);
						playerMovement.UpdateJumpTime(playerInputData.movement->jumpTime);
						playerMovement.UpdateWasJumpingState(playerInputData.movement->wasJumping);

						playerPhysics.SetFriction(playerInputData.movement->friction);
						playerPhysics.SetSurfaceVelocity(playerInputData.movement->surfaceVelocity);
					}
				}
			}

			m_prediction->Tick();

#ifdef DEBUG_PREDICTION
			for (std::size_t i = 0; i < m_playerData.size(); ++i)
			{
				auto& controllerData = m_playerData[i];
				if (controllerData.controlledEntity)
				{
					const Ndk::EntityHandle& reconciliationEntity = m_prediction->GetEntity(controllerData.controlledEntity->GetId());
					assert(reconciliationEntity);

					auto& reconciliationPhys = reconciliationEntity->GetComponent<Ndk::PhysicsComponent2D>();

					debugFile << "new position: " << reconciliationPhys.GetPosition().y << "\n";
					debugFile << "new velocity: " << reconciliationPhys.GetVelocity().y << "\n";
				}
			}
#endif
		}

#ifdef DEBUG_PREDICTION
		for (std::size_t i = 0; i < m_playerData.size(); ++i)
		{
			auto& controllerData = m_playerData[i];
			if (controllerData.controlledEntity)
			{
				const Ndk::EntityHandle& reconciliationEntity = m_prediction->GetEntity(controllerData.controlledEntity->GetId());
				assert(reconciliationEntity);

				auto& reconciliationPhys = reconciliationEntity->GetComponent<Ndk::PhysicsComponent2D>();

				debugFile << "--\n";
				debugFile << "final position: " << reconciliationPhys.GetPosition().y << "\n";
				debugFile << "final velocity: " << reconciliationPhys.GetVelocity().y << "\n";
			}
		}
#endif

		// Apply back predicted entities states to main world
		for (std::size_t i = 0; i < m_playerData.size(); ++i)
		{
			auto& controllerData = m_playerData[i];
			if (controllerData.controlledEntity)
			{
				const Ndk::EntityHandle& reconciliationEntity = m_prediction->GetEntity(controllerData.controlledEntity->GetId());
				if (!reconciliationEntity)
					continue;

				if (controllerData.controlledEntity->HasComponent<Ndk::PhysicsComponent2D>())
				{
					auto& realPhys = controllerData.controlledEntity->GetComponent<Ndk::PhysicsComponent2D>();
					auto& reconciliationPhys = reconciliationEntity->GetComponent<Ndk::PhysicsComponent2D>();

					Nz::Vector2f positionError = realPhys.GetPosition() - reconciliationPhys.GetPosition();

					if (positionError.GetSquaredLength() < Nz::IntegralPow(100, 2))
						realPhys.SetPosition(Nz::Lerp(realPhys.GetPosition(), reconciliationPhys.GetPosition(), 0.1f));
					else
					{
						std::cout << "Teleport!" << std::endl;
						realPhys.SetPosition(reconciliationPhys.GetPosition());
					}

					realPhys.SetAngularVelocity(reconciliationPhys.GetAngularVelocity());
					realPhys.SetRotation(reconciliationPhys.GetRotation());
					realPhys.SetVelocity(reconciliationPhys.GetVelocity());
				}
				else
				{
					auto& realNode = controllerData.controlledEntity->GetComponent<Ndk::NodeComponent>();
					auto& reconciliationNode = reconciliationEntity->GetComponent<Ndk::NodeComponent>();

					realNode.SetPosition(reconciliationNode.GetPosition());
					realNode.SetRotation(reconciliationNode.GetRotation());
				}
			}
		}
	}

	void LocalMatch::HandleTickPacket(Packets::PlayerWeapons&& packet)
	{
		auto& playerData = m_playerData[packet.playerIndex];
		playerData.weapons.clear();
		for (Nz::UInt32 weaponEntityIndex : packet.weaponEntities)
		{
			auto it = m_serverEntityIdToClient.find(weaponEntityIndex);
			assert(it != m_serverEntityIdToClient.end());

			ServerEntity& serverEntity = it.value();
			assert(serverEntity.entity); //< TODO: Change to if + continue (in case client failed to create entity)

			assert(serverEntity.entity->HasComponent<WeaponComponent>());

			playerData.weapons.emplace_back(serverEntity.entity);

			auto& scriptComponent = serverEntity.entity->GetComponent<ScriptComponent>();
			std::cout << "Local player #" << +packet.playerIndex << " has weapon " << scriptComponent.GetElement()->fullName << std::endl;
		}

		playerData.selectedWeapon = playerData.weapons.size();
	}

	void LocalMatch::HandleTickError(Nz::UInt16 stateTick, Nz::Int32 tickError)
	{
		for (auto it = m_tickPredictions.begin(); it != m_tickPredictions.end(); ++it)
		{
			if (it->serverTick == stateTick)
			{
				m_averageTickError.InsertValue(it->tickError + tickError);
				m_tickPredictions.erase(it);
				return;
			}
		}

		std::cout << "input not found: " << stateTick << std::endl;

		//m_averageTickError.InsertValue(m_averageTickError.GetAverageValue() + tickError);

		/*std::cout << "----" << std::endl;
		std::cout << "Current tick error: " << m_tickError << std::endl;
		std::cout << "Target tick error: " << tickError << std::endl;
		m_tickError = Nz::Approach(m_tickError, m_tickError + tickError, std::abs(std::max(10, 1)));
		std::cout << "New tick error: " << m_tickError << std::endl;*/
	}

	void LocalMatch::OnTick(bool lastTick)
	{
		Nz::UInt16 estimatedServerTick = GetNetworkTick(EstimateServerTick());

		Nz::UInt16 handledTick = AdjustServerTick(estimatedServerTick); //< To handle network jitter

		auto it = m_tickedPackets.begin();
		while (it != m_tickedPackets.end() && (it->tick == handledTick || IsMoreRecent(handledTick, it->tick)))
		{
			HandleTickPacket(std::move(it->content));
			++it;
		}
		m_tickedPackets.erase(m_tickedPackets.begin(), it);

		if (lastTick)
		{
			SendInputs(estimatedServerTick, true);

			// Remember predicted ticks for improving over time
			if (m_tickPredictions.size() >= static_cast<std::size_t>(std::ceil(2 / GetTickDuration()))) //< Remember at most 2s of inputs
				m_tickPredictions.erase(m_tickPredictions.begin());

			auto& prediction = m_tickPredictions.emplace_back();
			prediction.serverTick = estimatedServerTick;
			prediction.tickError = m_averageTickError.GetAverageValue();

			// Remember inputs for reconciliation
			PredictedInput& predictedInputs = m_predictedInputs.emplace_back();
			predictedInputs.serverTick = estimatedServerTick;

			predictedInputs.inputs.resize(m_playerData.size());
			for (std::size_t i = 0; i < m_playerData.size(); ++i)
			{
				auto& controllerData = m_playerData[i];

				auto& playerData = predictedInputs.inputs[i];
				playerData.input = controllerData.lastInputData;

				if (controllerData.controlledEntity && controllerData.controlledEntity->HasComponent<PlayerMovementComponent>())
				{
					auto& playerMovement = controllerData.controlledEntity->GetComponent<PlayerMovementComponent>();
					auto& playerPhysics = controllerData.controlledEntity->GetComponent<Ndk::PhysicsComponent2D>();

					auto& movementData = playerData.movement.emplace();
					movementData.isOnGround = playerMovement.IsOnGround();
					movementData.jumpTime = playerMovement.GetJumpTime();
					movementData.wasJumping = playerMovement.WasJumping();

					movementData.friction = playerPhysics.GetFriction();
					movementData.surfaceVelocity = playerPhysics.GetSurfaceVelocity();
				}

				if (controllerData.controlledEntity && controllerData.controlledEntity->HasComponent<InputComponent>())
				{
					auto& entityInputs = controllerData.controlledEntity->GetComponent<InputComponent>();
					entityInputs.UpdateInputs(controllerData.lastInputData);
				}
			}
		}

		if (m_gamemode)
			m_gamemode->ExecuteCallback("OnTick");

		Ndk::World& world = m_world.GetWorld();
		world.Update(GetTickDuration());

#ifdef DEBUG_PREDICTION
		ForEachEntity([&](const Ndk::EntityHandle& entity)
		{
			if (entity->HasComponent<InputComponent>())
			{
				auto& entityPhys = entity->GetComponent<Ndk::PhysicsComponent2D>();
				
				static std::ofstream debugFile("client.csv", std::ios::trunc);
				debugFile << m_application.GetAppTime() << ";" << ((entity->GetComponent<InputComponent>().GetInputs().isJumping) ? "Jumping;" : ";") << estimatedServerTick << ";" << entityPhys.GetPosition().y << ";" << entityPhys.GetVelocity().y << '\n';
			}
		});
#endif
	}

	void LocalMatch::PrepareClientUpdate()
	{
		Ndk::World& world = m_world.GetWorld();

		world.ForEachSystem([](Ndk::BaseSystem& system)
		{
			system.Enable(false);
		});

		world.GetSystem<Ndk::DebugSystem>().Enable(true);
		world.GetSystem<Ndk::ListenerSystem>().Enable(true);
		world.GetSystem<Ndk::ParticleSystem>().Enable(true);
		world.GetSystem<Ndk::RenderSystem>().Enable(true);
		world.GetSystem<AnimationSystem>().Enable(true);
		world.GetSystem<SoundSystem>().Enable(true);
	}

	void LocalMatch::PrepareTickUpdate()
	{
		Ndk::World& world = m_world.GetWorld();

		world.ForEachSystem([](Ndk::BaseSystem& system)
		{
			system.Enable(false);
		});

		world.GetSystem<Ndk::LifetimeSystem>().Enable(true);
		world.GetSystem<Ndk::PhysicsSystem2D>().Enable(true);
		world.GetSystem<Ndk::PhysicsSystem3D>().Enable(true);
		world.GetSystem<Ndk::VelocitySystem>().Enable(true);
		world.GetSystem<PlayerMovementSystem>().Enable(true);
		world.GetSystem<TickCallbackSystem>().Enable(true);
		world.GetSystem<WeaponSystem>().Enable(true);
	}

	void LocalMatch::ProcessInputs(float elapsedTime)
	{
		/*constexpr float MaxInputSendInterval = 1.f / 60.f;
		constexpr float MinInputSendInterval = 1.f / 10.f;

		m_playerInputTimer += elapsedTime;
		m_timeSinceLastInputSending += elapsedTime;

		bool inputUpdated = false;

		if (m_playerInputTimer >= MaxInputSendInterval)
		{
			m_playerInputTimer -= MaxInputSendInterval;
			if (SendInputs(TODO,m_timeSinceLastInputSending >= MinInputSendInterval))
				inputUpdated = true;
		}

		if (inputUpdated)
		{
			m_timeSinceLastInputSending = 0.f;

			// Remember inputs for reconciliation
			PredictedInput predictedInputs;
			predictedInputs.serverTick = m_inputPacket.estimatedServerTick;

			predictedInputs.inputs.resize(m_playerData.size());
			for (std::size_t i = 0; i < m_playerData.size(); ++i)
			{
				auto& controllerData = m_playerData[i];

				// Remember and apply inputs
				predictedInputs.inputs[i] = controllerData.lastInputData;

				if (controllerData.controlledEntity && controllerData.controlledEntity->HasComponent<InputComponent>())
				{
					auto& entityInputs = controllerData.controlledEntity->GetComponent<InputComponent>();
					entityInputs.UpdateInputs(controllerData.lastInputData);
				}
			}

			m_prediction.predictedInputs.emplace_back(std::move(predictedInputs));
		}*/
	}

	void LocalMatch::PushTickPacket(Nz::UInt16 tick, TickPacketContent&& packet)
	{
		TickPacket newPacket;
		newPacket.tick = tick;
		newPacket.content = std::move(packet);

		auto it = std::upper_bound(m_tickedPackets.begin(), m_tickedPackets.end(), newPacket, [](const TickPacket& a, const TickPacket& b)
		{
			return IsMoreRecent(b.tick, a.tick);
		});

		m_tickedPackets.emplace(it, std::move(newPacket));
	}

	bool LocalMatch::SendInputs(Nz::UInt16 serverTick, bool force)
	{
		assert(m_playerData.size() == m_inputPacket.inputs.size());

		m_inputPacket.estimatedServerTick = serverTick;

		bool hasInputData = false;
		for (std::size_t i = 0; i < m_playerData.size(); ++i)
		{
			auto& controllerData = m_playerData[i];
			PlayerInputData input;
			
			if (!m_chatBox.IsTyping() && (!m_console || !m_console->IsVisible()) && m_window->HasFocus())
				input = m_inputController->Poll(*this, controllerData.playerIndex, controllerData.controlledEntity);

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
