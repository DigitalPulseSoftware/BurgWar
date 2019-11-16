// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/LocalMatch.hpp>
#include <ClientLib/ClientSession.hpp>
#include <ClientLib/KeyboardAndMouseController.hpp>
#include <ClientLib/InputController.hpp>
#include <ClientLib/LocalCommandStore.hpp>
#include <ClientLib/VisualEntity.hpp>
#include <ClientLib/Scripting/ClientEditorScriptingLibrary.hpp>
#include <ClientLib/Scripting/ClientElementLibrary.hpp>
#include <ClientLib/Scripting/ClientEntityLibrary.hpp>
#include <ClientLib/Scripting/ClientGamemode.hpp>
#include <ClientLib/Scripting/ClientScriptingLibrary.hpp>
#include <ClientLib/Scripting/ClientWeaponLibrary.hpp>
#include <ClientLib/Components/LocalMatchComponent.hpp>
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
#include <Nazara/Math/Angle.hpp>
#include <Nazara/Network/Algorithm.hpp>
#include <Nazara/Renderer/DebugDrawer.hpp>
#include <Nazara/Platform/Keyboard.hpp>
#include <Nazara/Utility/SimpleTextDrawer.hpp>
#include <NDK/Components.hpp>
#include <NDK/Systems.hpp>
#include <cassert>
#include <fstream>

namespace bw
{
	namespace
	{
		Nz::UInt64 BuildEntityId(Nz::UInt16 layerIndex, Nz::UInt32 entityId)
		{
			return Nz::UInt64(layerIndex) << 32 | entityId;
		}
	}

	LocalMatch::LocalMatch(BurgApp& burgApp, Nz::RenderWindow* window, Ndk::Canvas* canvas, ClientSession& session, const Packets::MatchData& matchData) :
	SharedMatch(burgApp, LogSide::Client, "local", matchData.tickDuration),
	m_gamemodePath(matchData.gamemodePath),
	m_averageTickError(20),
	m_canvas(canvas),
	m_renderWorld(false),
	m_window(window),
	m_activeLayerIndex(0xFFFF),
	m_application(burgApp),
	m_chatBox(GetLogger(), window, canvas),
	m_session(session),
	m_errorCorrectionTimer(0.f),
	m_playerEntitiesTimer(0.f),
	m_playerInputTimer(0.f)
	{
		assert(window);

		m_averageTickError.InsertValue(-static_cast<Nz::Int32>(matchData.currentTick));

		m_layers.reserve(matchData.layers.size());

		LayerIndex layerIndex = 0;
		for (auto&& layerData : matchData.layers)
			m_layers.emplace_back(std::make_unique<LocalLayer>(*this, layerIndex++, layerData.backgroundColor));

		m_renderWorld.AddSystem<Ndk::DebugSystem>();
		m_renderWorld.AddSystem<Ndk::ListenerSystem>();
		m_renderWorld.AddSystem<Ndk::ParticleSystem>();
		m_renderWorld.AddSystem<Ndk::RenderSystem>();
		m_renderWorld.AddSystem<AnimationSystem>(*this);
		m_renderWorld.AddSystem<SoundSystem>();

		m_colorBackground = Nz::ColorBackground::New(Nz::Color::Black);

		Ndk::RenderSystem& renderSystem = m_renderWorld.GetSystem<Ndk::RenderSystem>();
		renderSystem.SetGlobalUp(Nz::Vector3f::Down());
		renderSystem.SetDefaultBackground(m_colorBackground);

		m_camera = m_renderWorld.CreateEntity();
		m_camera->AddComponent<Ndk::NodeComponent>();

		Ndk::CameraComponent& viewer = m_camera->AddComponent<Ndk::CameraComponent>();
		viewer.SetTarget(window);
		viewer.SetProjectionType(Nz::ProjectionType_Orthogonal);

		InitializeRemoteConsole();

		constexpr Nz::UInt8 playerCount = 1;

		m_inputPacket.inputs.resize(playerCount);
		for (auto& input : m_inputPacket.inputs)
			input.emplace();

		m_playerData.reserve(playerCount);
		assert(playerCount != 0xFF);
		for (Nz::UInt8 i = 0; i < playerCount; ++i)
		{
			auto& playerData = m_playerData.emplace_back(i);
			playerData.inputController = std::make_shared<KeyboardAndMouseController>(*window, i);
			
			playerData.inputController->OnSwitchWeapon.Connect([this, i](InputController* /*emitter*/, bool direction)
			{
				auto& playerData = m_playerData[i];

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
				selectPacket.playerIndex = i;
				selectPacket.newWeaponIndex = static_cast<Nz::UInt8>((playerData.selectedWeapon < playerData.weapons.size()) ? playerData.selectedWeapon : selectPacket.NoWeapon);

				m_session.SendPacket(selectPacket);
			});
		}

		m_prediction.emplace(GetTickDuration());

		m_chatBox.OnChatMessage.Connect([this](const std::string& message)
		{
			Packets::PlayerChat chatPacket;
			chatPacket.playerIndex = 0;
			chatPacket.message = message;

			m_session.SendPacket(chatPacket);
		});

		m_onUnhandledKeyPressed.Connect(canvas->OnUnhandledKeyPressed, [this](const Nz::EventHandler*, const Nz::WindowEvent::KeyEvent& event)
		{
			switch (event.code)
			{
				case Nz::Keyboard::F9:
					if (m_remoteConsole)
						m_remoteConsole->Hide();

					if (m_localConsole)
						m_localConsole->Show(!m_localConsole->IsVisible());

					break;

				case Nz::Keyboard::F10:
					if (m_localConsole)
						m_localConsole->Hide();

					if (m_remoteConsole)
						m_remoteConsole->Show(!m_remoteConsole->IsVisible());

					break;

				case Nz::Keyboard::Return:
					m_chatBox.Open(!m_chatBox.IsOpen());
					break;

				default:
					break;
			}
		});
		BindPackets();

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
					bwLog(GetLogger(), LogLevel::Info, "Debug socket bound to port {0}", m_debug->socket.GetBoundPort());
				}
				else
				{
					bwLog(GetLogger(), LogLevel::Error, "Failed to bind debug socket: {0}", Nz::ErrorToString(m_debug->socket.GetLastError()));
					m_debug.reset();
				}
			}
			else
			{
				bwLog(GetLogger(), LogLevel::Error, "Failed to create debug socket: {0}", Nz::ErrorToString(m_debug->socket.GetLastError()));
				m_debug.reset();
			}
		}
	}

	LocalMatch::~LocalMatch()
	{
		// Clear timer manager before scripting context gets deleted
		GetTimerManager().Clear();

		// Release scripts classes before scripting context
		m_entityStore.reset();
		m_weaponStore.reset();
		m_gamemode.reset();
	}

	void LocalMatch::ForEachEntity(std::function<void(const Ndk::EntityHandle& entity)> func)
	{
		for (auto& layer : m_layers)
		{
			if (layer->IsEnabled())
				layer->ForEachEntity(func);
		}
	}

	ClientEntityStore& LocalMatch::GetEntityStore()
	{
		assert(m_entityStore);
		return *m_entityStore;
	}

	const ClientEntityStore& LocalMatch::GetEntityStore() const
	{
		assert(m_entityStore);
		return *m_entityStore;
	}

	LocalLayer& LocalMatch::GetLayer(LayerIndex layerIndex)
	{
		assert(layerIndex < m_layers.size());
		return *m_layers[layerIndex];
	}

	const LocalLayer& LocalMatch::GetLayer(LayerIndex layerIndex) const
	{
		assert(layerIndex < m_layers.size());
		return *m_layers[layerIndex];
	}

	LayerIndex LocalMatch::GetLayerCount() const
	{
		return LayerIndex(m_layers.size());
	}

	ClientWeaponStore& LocalMatch::GetWeaponStore()
	{
		return *m_weaponStore;
	}

	const ClientWeaponStore& LocalMatch::GetWeaponStore() const
	{
		return *m_weaponStore;
	}

	void LocalMatch::LoadAssets(std::shared_ptr<VirtualDirectory> assetDir)
	{
		if (!m_assetStore)
			m_assetStore.emplace(GetLogger(), std::move(assetDir));
		else
		{
			m_assetStore->UpdateAssetDirectory(std::move(assetDir));
			m_assetStore->Clear();
		}
	}

	void LocalMatch::LoadScripts(const std::shared_ptr<VirtualDirectory>& scriptDir)
	{
		assert(m_assetStore);

		if (!m_scriptingContext)
		{
			std::shared_ptr<ClientScriptingLibrary> scriptingLibrary = std::make_shared<ClientScriptingLibrary>(*this);

			m_scriptingContext = std::make_shared<ScriptingContext>(GetLogger(), scriptDir);
			m_scriptingContext->LoadLibrary(scriptingLibrary);
			m_scriptingContext->LoadLibrary(std::make_shared<ClientEditorScriptingLibrary>(GetLogger(), *m_assetStore));

			if (!m_localConsole)
				m_localConsole.emplace(GetLogger(), m_window, m_canvas, scriptingLibrary, scriptDir);
		}
		else
		{
			m_scriptingContext->UpdateScriptDirectory(scriptDir);
			m_scriptingContext->ReloadLibraries();
		}

		std::shared_ptr<ClientElementLibrary> clientElementLib;

		if (!m_entityStore)
		{
			if (!clientElementLib)
				clientElementLib = std::make_shared<ClientElementLibrary>(GetLogger());

			m_entityStore.emplace(*m_assetStore, GetLogger(), m_scriptingContext);
			m_entityStore->LoadLibrary(clientElementLib);
			m_entityStore->LoadLibrary(std::make_shared<ClientEntityLibrary>(GetLogger(), *m_assetStore));
		}
		else
		{
			m_entityStore->ClearElements();
			m_entityStore->ReloadLibraries();
		}

		if (!m_weaponStore)
		{
			if (!clientElementLib)
				clientElementLib = std::make_shared<ClientElementLibrary>(GetLogger());

			m_weaponStore.emplace(*m_assetStore, GetLogger(), m_scriptingContext);
			m_weaponStore->LoadLibrary(clientElementLib);
			m_weaponStore->LoadLibrary(std::make_shared<ClientWeaponLibrary>(GetLogger(), *m_assetStore));
		}
		else
		{
			m_weaponStore->ClearElements();
			m_weaponStore->ReloadLibraries();
		}

		VirtualDirectory::Entry entry;

		if (scriptDir->GetEntry("entities", &entry))
		{
			std::filesystem::path path = "entities";

			VirtualDirectory::VirtualDirectoryEntry& directory = std::get<VirtualDirectory::VirtualDirectoryEntry>(entry);
			directory->Foreach([&](const std::string& entryName, const VirtualDirectory::Entry& entry)
			{
				m_entityStore->LoadElement(std::holds_alternative<VirtualDirectory::VirtualDirectoryEntry>(entry), path / entryName);
			});
		}

		if (scriptDir->GetEntry("weapons", &entry))
		{
			std::filesystem::path path = "weapons";

			VirtualDirectory::VirtualDirectoryEntry& directory = std::get<VirtualDirectory::VirtualDirectoryEntry>(entry);
			directory->Foreach([&](const std::string& entryName, const VirtualDirectory::Entry& entry)
			{
				m_weaponStore->LoadElement(std::holds_alternative<VirtualDirectory::VirtualDirectoryEntry>(entry), path / entryName);
			});
		}

		sol::state& state = m_scriptingContext->GetLuaState();
		state["engine_AnimateRotation"] = [&](const sol::table& entityTable, float fromAngle, float toAngle, float duration, sol::object callbackObject)
		{
			const Ndk::EntityHandle& entity = AbstractElementLibrary::AssertScriptEntity(entityTable);

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
					bwLog(GetLogger(), LogLevel::Error, "engine_AnimateRotation callback failed: {0}", err.what());
				}
			});
			return 0;
		};

		state["engine_AnimatePositionByOffsetSq"] = [&](const sol::table& entityTable, const Nz::Vector2f& fromOffset, const Nz::Vector2f& toOffset, float duration, sol::object callbackObject)
		{
			const Ndk::EntityHandle& entity = AbstractElementLibrary::AssertScriptEntity(entityTable);

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
					bwLog(GetLogger(), LogLevel::Error, "engine_AnimatePositionByOffset callback failed: {0}", err.what());
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
				return sol::make_object(lua, playerData.controlledEntity->GetPosition());
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

			OnCameraMoved(this, position);
		};

		state["engine_OverridePlayerInputController"] = [&](Nz::UInt8 playerIndex, std::shared_ptr<InputController> inputController)
		{
			if (playerIndex >= m_playerData.size())
				throw std::runtime_error("Invalid player index");

			if (!inputController)
				throw std::runtime_error("Invalid input controller");

			m_playerData[playerIndex].inputController = std::move(inputController);
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

		SharedMatch::Update(elapsedTime);

		/*constexpr float ErrorCorrectionPerSecond = 60;

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
		}*/

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

							/*if (auto it = m_serverEntityIdToClient.find(entityId); it != m_serverEntityIdToClient.end())
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
							}*/
						}

						break;
					}

					default:
						break;
				}
			}
		}

		for (auto& layerPtr : m_layers)
		{
			if (layerPtr->IsEnabled())
				layerPtr->SyncVisuals();
		}

		m_animationManager.Update(elapsedTime);
		if (m_gamemode)
			m_gamemode->ExecuteCallback("OnFrame");

		m_renderWorld.Update(elapsedTime);

		/*Ndk::PhysicsSystem2D::DebugDrawOptions options;
		options.polygonCallback = [](const Nz::Vector2f* vertices, std::size_t vertexCount, float radius, Nz::Color outline, Nz::Color fillColor, void* userData)
		{
			for (std::size_t i = 0; i < vertexCount - 1; ++i)
				Nz::DebugDrawer::DrawLine(vertices[i], vertices[i + 1]);

			Nz::DebugDrawer::DrawLine(vertices[vertexCount - 1], vertices[0]);
		};

		m_world.GetWorld().GetSystem<Ndk::PhysicsSystem2D>().DebugDraw(options);*/
	}
	
	void LocalMatch::BindPackets()
	{
		//TODO: Use slots
		m_session.OnChatMessage.Connect([this](ClientSession* /*session*/, const Packets::ChatMessage& message)
		{
			HandleChatMessage(message);
		});

		m_session.OnConsoleAnswer.Connect([this](ClientSession* /*session*/, const Packets::ConsoleAnswer& consoleAnswer)
		{
			HandleConsoleAnswer(consoleAnswer);
		});

		m_session.OnControlEntity.Connect([this](ClientSession* /*session*/, const Packets::ControlEntity& controlEntity)
		{
			PushTickPacket(controlEntity.stateTick, controlEntity);
		});

		m_session.OnCreateEntities.Connect([this](ClientSession* /*session*/, const Packets::CreateEntities& createEntities)
		{
			PushTickPacket(createEntities.stateTick, createEntities);
		});

		m_session.OnDeleteEntities.Connect([this](ClientSession* /*session*/, const Packets::DeleteEntities& deleteEntities)
		{
			PushTickPacket(deleteEntities.stateTick, deleteEntities);
		});
		
		m_session.OnDisableLayer.Connect([this](ClientSession* /*session*/, const Packets::DisableLayer& disableLayer)
		{
			PushTickPacket(disableLayer.stateTick, disableLayer);
		});
		
		m_session.OnEnableLayer.Connect([this](ClientSession* /*session*/, const Packets::EnableLayer& enableLayer)
		{
			PushTickPacket(enableLayer.stateTick, enableLayer);
		});

		m_session.OnEntitiesAnimation.Connect([this](ClientSession* /*session*/, const Packets::EntitiesAnimation& animations)
		{
			PushTickPacket(animations.stateTick, animations);
		});

		m_session.OnEntitiesDeath.Connect([this](ClientSession* /*session*/, const Packets::EntitiesDeath& deaths)
		{
			PushTickPacket(deaths.stateTick, deaths);
		});

		m_session.OnEntitiesInputs.Connect([this](ClientSession* /*session*/, const Packets::EntitiesInputs& inputs)
		{
			PushTickPacket(inputs.stateTick, inputs);
		});

		m_session.OnEntityWeapon.Connect([this](ClientSession* /*session*/, const Packets::EntityWeapon& weapon)
		{
			PushTickPacket(weapon.stateTick, weapon);
		});

		m_session.OnHealthUpdate.Connect([this](ClientSession* /*session*/, const Packets::HealthUpdate& healthUpdate)
		{
			PushTickPacket(healthUpdate.stateTick, healthUpdate);
		});

		m_session.OnInputTimingCorrection.Connect([this](ClientSession* /*session*/, const Packets::InputTimingCorrection& timingCorrection)
		{
			HandleTickError(timingCorrection.serverTick, timingCorrection.tickError);
		});

		m_session.OnMatchState.Connect([this](ClientSession* /*session*/, const Packets::MatchState& matchState)
		{
			PushTickPacket(matchState.stateTick, matchState);
		});

		m_session.OnPlayerLayer.Connect([this](ClientSession* /*session*/, const Packets::PlayerLayer& layerUpdate)
		{
			PushTickPacket(layerUpdate.stateTick, layerUpdate);
		});

		m_session.OnPlayerWeapons.Connect([this](ClientSession* /*session*/, const Packets::PlayerWeapons& weapons)
		{
			PushTickPacket(weapons.stateTick, weapons);
		});
	}

	Nz::UInt64 LocalMatch::EstimateServerTick() const
	{
		return GetCurrentTick() - m_averageTickError.GetAverageValue();
	}

	void LocalMatch::HandleChatMessage(const Packets::ChatMessage& packet)
	{
		m_chatBox.PrintMessage(packet.content);
	}

	void LocalMatch::HandleConsoleAnswer(const Packets::ConsoleAnswer& packet)
	{
		if (m_remoteConsole)
			m_remoteConsole->Print(packet.response, packet.color);
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
		assert(packet.layerIndex < m_layers.size());
		auto& layerPtr = m_layers[packet.layerIndex];

		auto layerEntityOpt = layerPtr->GetEntity(packet.entityId);
		if (!layerEntityOpt)
			return;

		LocalLayerEntity& layerEntity = layerEntityOpt.value();

		if (m_playerData[packet.playerIndex].controlledEntity)
			m_playerData[packet.playerIndex].controlledEntity->GetEntity()->RemoveComponent<Ndk::ListenerComponent>();

		m_playerData[packet.playerIndex].controlledEntity = layerEntity.CreateHandle();
		m_playerData[packet.playerIndex].controlledEntity->GetEntity()->AddComponent<Ndk::ListenerComponent>();
	}

	void LocalMatch::HandleTickPacket(Packets::CreateEntities&& packet)
	{
		std::size_t offset = 0;
		for (auto&& layerData : packet.layers)
		{
			assert(layerData.layerIndex < m_layers.size());
			auto& layer = m_layers[layerData.layerIndex];
			layer->HandlePacket(&packet.entities[offset], layerData.entityCount);
			offset += layerData.entityCount;
		}
	}

	void LocalMatch::HandleTickPacket(Packets::DeleteEntities&& packet)
	{
		std::size_t offset = 0;
		for (auto&& layerData : packet.layers)
		{
			assert(layerData.layerIndex < m_layers.size());
			auto& layer = m_layers[layerData.layerIndex];
			layer->HandlePacket(&packet.entities[offset], layerData.entityCount);
			offset += layerData.entityCount;
		}
	}

	void LocalMatch::HandleTickPacket(Packets::DisableLayer&& packet)
	{
		assert(m_layers[packet.layerIndex]->IsEnabled());
		bwLog(GetLogger(), LogLevel::Debug, "Layer {} is now disabled", packet.layerIndex);

		//TODO
		m_layers[packet.layerIndex]->Disable();
	}

	void LocalMatch::HandleTickPacket(Packets::EnableLayer&& packet)
	{
		assert(!m_layers[packet.layerIndex]->IsEnabled());
		bwLog(GetLogger(), LogLevel::Debug, "Layer {} is now enabled", packet.layerIndex);

		//TODO
		auto& layer = m_layers[packet.layerIndex];
		layer->Enable();
		layer->HandlePacket(packet.layerEntities.data(), packet.layerEntities.size());
	}

	void LocalMatch::HandleTickPacket(Packets::EntitiesAnimation&& packet)
	{
		std::size_t offset = 0;
		for (auto&& layerData : packet.layers)
		{
			assert(layerData.layerIndex < m_layers.size());
			auto& layer = m_layers[layerData.layerIndex];
			layer->HandlePacket(&packet.entities[offset], layerData.entityCount);
			offset += layerData.entityCount;
		}
	}

	void LocalMatch::HandleTickPacket(Packets::EntitiesDeath&& packet)
	{
		std::size_t offset = 0;
		for (auto&& layerData : packet.layers)
		{
			assert(layerData.layerIndex < m_layers.size());
			auto& layer = m_layers[layerData.layerIndex];
			layer->HandlePacket(&packet.entities[offset], layerData.entityCount);
			offset += layerData.entityCount;
		}
	}

	void LocalMatch::HandleTickPacket(Packets::EntitiesInputs&& packet)
	{
		std::size_t offset = 0;
		for (auto&& layerData : packet.layers)
		{
			assert(layerData.layerIndex < m_layers.size());
			auto& layer = m_layers[layerData.layerIndex];
			layer->HandlePacket(&packet.entities[offset], layerData.entityCount);
			offset += layerData.entityCount;
		}
	}

	void LocalMatch::HandleTickPacket(Packets::EntityWeapon&& packet)
	{
		assert(packet.layerIndex < m_layers.size());
		auto& layer = m_layers[packet.layerIndex];

		auto entityOpt = layer->GetEntity(packet.entityId);
		if (!entityOpt)
			return;

		LocalLayerEntity& entity = entityOpt.value();

		if (packet.weaponEntityId != Packets::EntityWeapon::NoWeapon)
		{
			auto newWeaponOpt = layer->GetEntity(packet.weaponEntityId);
			if (!newWeaponOpt)
				return;

			LocalLayerEntity& newWeapon = newWeaponOpt.value();

			entity.UpdateWeaponEntity(newWeapon.CreateHandle());
		}
		else
			entity.UpdateWeaponEntity({});
	}

	void LocalMatch::HandleTickPacket(Packets::HealthUpdate&& packet)
	{
		std::size_t offset = 0;
		for (auto&& layerData : packet.layers)
		{
			assert(layerData.layerIndex < m_layers.size());
			auto& layer = m_layers[layerData.layerIndex];
			layer->HandlePacket(&packet.entities[offset], layerData.entityCount);
			offset += layerData.entityCount;
		}
	}

	void LocalMatch::HandleTickPacket(Packets::MatchState&& packet)
	{
		std::size_t offset = 0;
		for (auto&& layerData : packet.layers)
		{
			assert(layerData.layerIndex < m_layers.size());
			auto& layer = m_layers[layerData.layerIndex];
			layer->HandlePacket(&packet.entities[offset], layerData.entityCount);
			offset += layerData.entityCount;
		}

		/*if (Nz::Keyboard::IsKeyPressed(Nz::Keyboard::A))
			return;

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

		for (auto&& entityData : packet.entities)
		{
			auto& layer = m_layers[entityData.id.layerId];
			Ndk::World& world = layer->GetWorld();
			auto& physicsSystem = world.GetSystem<Ndk::PhysicsSystem2D>();

			Nz::UInt64 entityKey = BuildEntityId(entityData.id.layerId, entityData.id.entityId);
			auto it = m_serverEntityIdToClient.find(entityKey);
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
				if (controllerData.controlledEntityServerId == entityKey && controllerData.controlledEntity)
				{
					isPredicted = true;

#ifdef DEBUG_PREDICTION
					if (serverEntity.entity->HasComponent<InputComponent>())
					{
						debugFile << "Burger position: " << entityData.position.y << "\n";
						debugFile << "Burger velocity: " << entityData.physicsProperties->linearVelocity.y << "\n";
					}
#endif

					m_prediction->RegisterForPrediction(BuildEntityId(entityData.id.layerId, serverEntity.entity->GetId()), controllerData.controlledEntity, [](const Ndk::EntityHandle& entity)
					{
						entity->AddComponent<InputComponent>();
						entity->AddComponent<PlayerMovementComponent>();
					}, [&](const Ndk::EntityHandle&, const Ndk::EntityHandle& targetEntity)
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

					auto& physicsSystem = controllerData.controlledEntity->GetWorld()->GetSystem<Ndk::PhysicsSystem2D>();
					LayerIndex layerIndex = controllerData.controlledEntity->GetComponent<LocalMatchComponent>().GetLayerIndex();

					physicsSystem.RegionQuery(Nz::Rectf(position.x - 500, position.y - 500, 1000.f, 1000.f), 0, 0xFFFFFFFF, 0xFFFFFFFF, [&](const Ndk::EntityHandle& entity)
					{
						// Prevent registering player entities a second time (as it would resync their physics which we don't want)
						Nz::UInt64 layerEntityId = BuildEntityId(layerIndex, entity->GetId());
						if (!m_prediction->IsRegistered(layerEntityId))
						{
							m_prediction->RegisterForPrediction(layerEntityId, entity, [](const Ndk::EntityHandle& entity)
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
					auto& controlledEntityMatch = controllerData.controlledEntity->GetComponent<LocalMatchComponent>();

					const Ndk::EntityHandle& reconciliationEntity = m_prediction->GetEntity(BuildEntityId(controlledEntityMatch.GetLayerIndex(), controllerData.controlledEntity->GetId()));
					if (!reconciliationEntity)
						continue;

					//assert(reconciliationEntity);

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
					auto& controlledEntityMatch = controllerData.controlledEntity->GetComponent<LocalMatchComponent>();

					const Ndk::EntityHandle& reconciliationEntity = m_prediction->GetEntity(BuildEntityId(controlledEntityMatch.GetLayerIndex(), controllerData.controlledEntity->GetId()));
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
				auto& controlledEntityMatch = controllerData.controlledEntity->GetComponent<LocalMatchComponent>();

				const Ndk::EntityHandle& reconciliationEntity = m_prediction->GetEntity(BuildEntityId(controlledEntityMatch.GetLayerIndex(), controllerData.controlledEntity->GetId()));
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
				auto& controlledEntityMatch = controllerData.controlledEntity->GetComponent<LocalMatchComponent>();

				const Ndk::EntityHandle& reconciliationEntity = m_prediction->GetEntity(BuildEntityId(controlledEntityMatch.GetLayerIndex(), controllerData.controlledEntity->GetId()));
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
						bwLog(GetLogger(), LogLevel::Warning, "Teleport!");
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
		}*/
	}

	void LocalMatch::HandleTickPacket(Packets::PlayerLayer&& packet)
	{
		m_playerData[packet.playerIndex].layerIndex = packet.layerIndex;

		m_activeLayerIndex = packet.layerIndex;

		auto& layer = m_layers[m_activeLayerIndex];
		assert(layer->IsEnabled());

		m_colorBackground->SetColor(layer->GetBackgroundColor());
		
		m_onEntityCreated.Connect(layer->OnEntityCreated, [&](LocalLayer*, LocalLayerEntity& newEntity)
		{
			m_visualEntities.emplace(newEntity.GetServerId(), VisualEntity(m_renderWorld, newEntity.CreateHandle()));
		});

		m_onEntityDelete.Connect(layer->OnEntityDelete, [&](LocalLayer*, LocalLayerEntity& newEntity)
		{
			m_visualEntities.erase(newEntity.GetServerId());
		});

		m_visualEntities.clear();
		layer->ForEachLayerEntity([&](LocalLayerEntity& layerEntity)
		{
			m_visualEntities.emplace(layerEntity.GetServerId(), VisualEntity(m_renderWorld, layerEntity.CreateHandle()));
		});

		//m_orderedLayers.clear();
		//m_orderedLayers.emplace_back(packet.layerIndex, 0);
		/*for (auto& visibilityInfo : packet.visibleLayers)
		{
			auto& layer = m_layers[visibilityInfo.layerIndex];

			Nz::Node& cameraNode = layer.GetCameraNode();
			cameraNode.SetPosition(Nz::Vector2f::Zero());
			cameraNode.SetRotation(Nz::RadianAnglef::Zero());
			cameraNode.SetScale(visibilityInfo.parallaxFactor);

			Nz::Node& node = layer.GetNode();
			node.SetPosition(visibilityInfo.offset);
			node.SetRotation(visibilityInfo.rotation);
			node.SetScale(visibilityInfo.scale);

			m_orderedLayers.emplace_back(visibilityInfo.layerIndex, visibilityInfo.renderOrder);
		}*/
		//std::sort(m_orderedLayers.begin(), m_orderedLayers.end(), [](auto&& lhs, auto&& rhs) { return lhs.second < rhs.second; });

		/*auto& currentLayer = m_layers[m_activeLayerIndex];

		Nz::Node& cameraNode = currentLayer->GetCameraNode();
		cameraNode.SetPosition(Nz::Vector2f::Zero());
		cameraNode.SetRotation(Nz::RadianAnglef::Zero());
		cameraNode.SetScale(Nz::Vector2f::Unit());

		Nz::Node& node = currentLayer->GetNode();
		node.SetPosition(Nz::Vector2f::Zero());
		node.SetRotation(Nz::RadianAnglef::Zero());
		node.SetScale(Nz::Vector2f::Unit());*/
	}

	void LocalMatch::HandleTickPacket(Packets::PlayerWeapons&& packet)
	{
		auto& playerData = m_playerData[packet.playerIndex];
		playerData.weapons.clear();

		assert(packet.layerIndex < m_layers.size());
		auto& layer = m_layers[packet.layerIndex];

		for (auto weaponEntityIndex : packet.weaponEntities)
		{
			auto entityOpt = layer->GetEntity(weaponEntityIndex);
			assert(entityOpt);

			LocalLayerEntity& layerEntity = entityOpt.value();

			assert(layerEntity.GetEntity()->HasComponent<WeaponComponent>());

			playerData.weapons.emplace_back(layerEntity.GetEntity());

			auto& scriptComponent = layerEntity.GetEntity()->GetComponent<ScriptComponent>();
			bwLog(GetLogger(), LogLevel::Info, "Local player #{0} has weapon {1}", +packet.playerIndex, scriptComponent.GetElement()->fullName);
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

		bwLog(GetLogger(), LogLevel::Warning, "Input not found for state tick {0}", stateTick);

		//m_averageTickError.InsertValue(m_averageTickError.GetAverageValue() + tickError);

		/*std::cout << "----" << std::endl;
		std::cout << "Current tick error: " << m_tickError << std::endl;
		std::cout << "Target tick error: " << tickError << std::endl;
		m_tickError = Nz::Approach(m_tickError, m_tickError + tickError, std::abs(std::max(10, 1)));
		std::cout << "New tick error: " << m_tickError << std::endl;*/
	}

	void LocalMatch::InitializeRemoteConsole()
	{
		m_remoteConsole.emplace(m_window, m_canvas);
		m_remoteConsole->SetExecuteCallback([this](const std::string& command) -> bool
		{
			Packets::PlayerConsoleCommand commandPacket;
			commandPacket.command = command;
			commandPacket.playerIndex = 0;

			m_session.SendPacket(commandPacket);

			return true;
		});
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

				if (controllerData.controlledEntity)
				{
					auto& entity = controllerData.controlledEntity->GetEntity();
					if (entity->HasComponent<PlayerMovementComponent>())
					{
						auto& playerMovement = entity->GetComponent<PlayerMovementComponent>();
						auto& playerPhysics = entity->GetComponent<Ndk::PhysicsComponent2D>();

						auto& movementData = playerData.movement.emplace();
						movementData.isOnGround = playerMovement.IsOnGround();
						movementData.jumpTime = playerMovement.GetJumpTime();
						movementData.wasJumping = playerMovement.WasJumping();

						movementData.friction = playerPhysics.GetFriction();
						movementData.surfaceVelocity = playerPhysics.GetSurfaceVelocity();
					}

					if (entity->HasComponent<InputComponent>())
					{
						auto& entityInputs = entity->GetComponent<InputComponent>();
						//entityInputs.UpdateInputs(controllerData.lastInputData);
					}
				}
			}
		}

		if (m_gamemode)
			m_gamemode->ExecuteCallback("OnTick");

		for (auto& layer : m_layers)
		{
			if (layer->IsEnabled())
				layer->Update(GetTickDuration());
		}

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

	void LocalMatch::PushTickPacket(Nz::UInt16 tick, const TickPacketContent& packet)
	{
		TickPacket newPacket;
		newPacket.tick = tick;
		newPacket.content = packet;

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
		
		bool checkInputs = !m_chatBox.IsTyping() &&
		                  (!m_localConsole || !m_localConsole->IsVisible()) &&
		                  (!m_remoteConsole || !m_remoteConsole->IsVisible()) &&
		                  (m_window->HasFocus());

		bool hasInputData = false;
		for (std::size_t i = 0; i < m_playerData.size(); ++i)
		{
			auto& controllerData = m_playerData[i];
			PlayerInputData input;

			if (checkInputs)
				input = controllerData.inputController->Poll(*this, controllerData.controlledEntity);

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
