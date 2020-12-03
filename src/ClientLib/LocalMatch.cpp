// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/LocalMatch.hpp>
#include <CoreLib/Components/AnimationComponent.hpp>
#include <CoreLib/Components/CooldownComponent.hpp>
#include <CoreLib/Components/InputComponent.hpp>
#include <CoreLib/Components/PlayerMovementComponent.hpp>
#include <CoreLib/Components/ScriptComponent.hpp>
#include <CoreLib/Components/WeaponComponent.hpp>
#include <CoreLib/Scripting/NetworkPacket.hpp>
#include <CoreLib/Scripting/ScriptingUtils.hpp>
#include <CoreLib/Systems/AnimationSystem.hpp>
#include <CoreLib/Systems/PlayerMovementSystem.hpp>
#include <CoreLib/Systems/TickCallbackSystem.hpp>
#include <CoreLib/Systems/WeaponSystem.hpp>
#include <ClientLib/ClientEditorApp.hpp>
#include <ClientLib/ClientSession.hpp>
#include <ClientLib/KeyboardAndMouseController.hpp>
#include <ClientLib/InputController.hpp>
#include <ClientLib/LocalCommandStore.hpp>
#include <ClientLib/Scoreboard.hpp>
#include <ClientLib/VisualEntity.hpp>
#include <ClientLib/Components/VisibleLayerComponent.hpp>
#include <ClientLib/Scripting/ClientEditorScriptingLibrary.hpp>
#include <ClientLib/Scripting/ClientElementLibrary.hpp>
#include <ClientLib/Scripting/ClientEntityLibrary.hpp>
#include <ClientLib/Scripting/ClientGamemode.hpp>
#include <ClientLib/Scripting/ClientScriptingLibrary.hpp>
#include <ClientLib/Scripting/ClientWeaponLibrary.hpp>
#include <ClientLib/Components/LocalMatchComponent.hpp>
#include <ClientLib/Systems/SoundSystem.hpp>
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
	LocalMatch::LocalMatch(ClientEditorApp& burgApp, Nz::RenderWindow* window, Nz::RenderTarget* renderTarget, Ndk::Canvas* canvas, ClientSession& session, const Packets::AuthSuccess& authSuccess, const Packets::MatchData& matchData) :
	SharedMatch(burgApp, LogSide::Client, "local", matchData.tickDuration),
	m_gamemodeName(matchData.gamemode),
	m_canvas(canvas),
	m_renderWorld(false),
	m_freeClientId(-1),
	m_renderTarget(renderTarget),
	m_window(window),
	m_activeLayerIndex(0xFFFF),
	m_averageTickError(20),
	m_chatBox(GetLogger(), renderTarget, canvas),
	m_application(burgApp),
	m_session(session),
	m_escapeMenu(burgApp, canvas),
	m_scoreboard(nullptr),
	m_hasFocus(window->HasFocus()),
	m_isLeavingMatch(false),
	m_errorCorrectionTimer(0.f),
	m_playerEntitiesTimer(0.f),
	m_playerInputTimer(0.f)
	{
		assert(window);

		for (const auto& property : matchData.gamemodeProperties)
		{
			const std::string& propertyName = m_session.GetNetworkStringStore().GetString(property.name);
			m_gamemodeProperties.emplace(propertyName, property.value);
		}

		m_averageTickError.InsertValue(-static_cast<Nz::Int32>(matchData.currentTick));

		m_layers.reserve(matchData.layers.size());

		LayerIndex layerIndex = 0;
		for (auto&& layerData : matchData.layers)
			m_layers.emplace_back(std::make_unique<LocalLayer>(*this, layerIndex++, layerData.backgroundColor));

		auto& playerSettings = burgApp.GetPlayerSettings();

		m_renderWorld.AddSystem<Ndk::DebugSystem>();
		m_renderWorld.AddSystem<Ndk::ListenerSystem>();
		m_renderWorld.AddSystem<Ndk::ParticleSystem>();
		m_renderWorld.AddSystem<Ndk::RenderSystem>();
		m_renderWorld.AddSystem<AnimationSystem>(*this);
		m_renderWorld.AddSystem<SoundSystem>(playerSettings);

		m_colorBackground = Nz::ColorBackground::New(Nz::Color::Black);

		Ndk::RenderSystem& renderSystem = m_renderWorld.GetSystem<Ndk::RenderSystem>();
		renderSystem.SetGlobalUp(Nz::Vector3f::Down());
		renderSystem.SetDefaultBackground(m_colorBackground);

		m_camera.emplace(m_renderWorld, renderTarget, true);
		m_camera->SetZoomFactor(0.8f);

		m_currentLayer = m_renderWorld.CreateEntity();
		m_currentLayer->AddComponent<Ndk::NodeComponent>();
		m_currentLayer->AddComponent<VisibleLayerComponent>(m_renderWorld);

		InitializeRemoteConsole();

		std::size_t playerCount = authSuccess.players.size();

		m_inputPacket.inputs.resize(playerCount);
		for (auto& input : m_inputPacket.inputs)
			input.emplace();

		m_localPlayers.reserve(playerCount);
		assert(playerCount != 0xFF);
		for (Nz::UInt8 i = 0; i < playerCount; ++i)
		{
			auto& playerData = m_localPlayers.emplace_back(i);
			playerData.inputController = std::make_shared<KeyboardAndMouseController>(*window, i);
			playerData.playerIndex = authSuccess.players[i].playerIndex;
			playerData.inputController->OnSwitchWeapon.Connect([this, i](InputController* /*emitter*/, bool direction)
			{
				auto& playerData = m_localPlayers[i];

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
				selectPacket.localIndex = i;
				selectPacket.newWeaponIndex = static_cast<Nz::UInt8>((playerData.selectedWeapon < playerData.weapons.size()) ? playerData.selectedWeapon : selectPacket.NoWeapon);

				m_session.SendPacket(selectPacket);
			});
		}

		BindEscapeMenu();
		BindPackets();
		BindSignals(burgApp, window, canvas);
	}

	LocalMatch::~LocalMatch()
	{
		// Clear timer manager before scripting context gets deleted
		GetScriptPacketHandlerRegistry().Clear();
		GetTimerManager().Clear();

		m_layers.clear();

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

	const NetworkStringStore& LocalMatch::GetNetworkStringStore() const
	{
		return m_session.GetNetworkStringStore();
	}

	std::shared_ptr<const SharedGamemode> LocalMatch::GetSharedGamemode() const
	{
		return m_gamemode;
	}

	ClientWeaponStore& LocalMatch::GetWeaponStore()
	{
		return *m_weaponStore;
	}

	const ClientWeaponStore& LocalMatch::GetWeaponStore() const
	{
		return *m_weaponStore;
	}

	void LocalMatch::InitDebugGhosts()
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

	void LocalMatch::LoadAssets(std::shared_ptr<VirtualDirectory> assetDir)
	{
		if (!m_assetStore)
		{
			m_assetStore.emplace(GetLogger(), std::move(assetDir));
			m_particleRegistry.emplace(m_assetStore.value());
		}
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
				m_localConsole.emplace(GetLogger(), m_renderTarget, m_canvas, scriptingLibrary, scriptDir);
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
				clientElementLib = std::make_shared<ClientElementLibrary>(GetLogger(), *m_assetStore);

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
				clientElementLib = std::make_shared<ClientElementLibrary>(GetLogger(), *m_assetStore);

			m_weaponStore.emplace(*m_assetStore, GetLogger(), m_scriptingContext);
			m_weaponStore->LoadLibrary(clientElementLib);
			m_weaponStore->LoadLibrary(std::make_shared<ClientWeaponLibrary>(GetLogger(), *m_assetStore));
		}
		else
		{
			m_weaponStore->ClearElements();
			m_weaponStore->ReloadLibraries();
		}

		m_entityStore->LoadDirectory("entities");
		m_entityStore->Resolve();

		m_weaponStore->LoadDirectory("weapons");
		m_weaponStore->Resolve();

		sol::state& state = m_scriptingContext->GetLuaState();
		state["engine_AnimateRotation"] = LuaFunction([&](const sol::table& entityTable, float fromAngle, float toAngle, float duration, sol::main_protected_function callback)
		{
			Ndk::EntityHandle entity = AssertScriptEntity(entityTable);

			m_animationManager.PushAnimation(duration, [=](float ratio)
			{
				if (!entity)
					return false;

				float newAngle = Nz::Lerp(fromAngle, toAngle, ratio);
				auto& nodeComponent = entity->GetComponent<Ndk::NodeComponent>();
				nodeComponent.SetRotation(Nz::DegreeAnglef(newAngle));

				return true;
			}, [this, callback]()
			{
				auto result = callback();
				if (!result.valid())
				{
					sol::error err = result;
					bwLog(GetLogger(), LogLevel::Error, "engine_AnimateRotation callback failed: {0}", err.what());
				}
			});
			return 0;
		});

		state["engine_AnimatePositionByOffsetSq"] = LuaFunction([&](const sol::table& entityTable, const Nz::Vector2f& fromOffset, const Nz::Vector2f& toOffset, float duration, sol::main_protected_function callback)
		{
			Ndk::EntityHandle entity = AssertScriptEntity(entityTable);

			m_animationManager.PushAnimation(duration, [=](float ratio)
			{
				if (!entity)
					return false;

				Nz::Vector2f offset = Nz::Lerp(fromOffset, toOffset, ratio * ratio); //< FIXME
				auto& nodeComponent = entity->GetComponent<Ndk::NodeComponent>();
				nodeComponent.SetInitialPosition(offset); //< FIXME

				return true;
			}, [this, callback]()
			{
				auto result = callback();
				if (!result.valid())
				{
					sol::error err = result;
					bwLog(GetLogger(), LogLevel::Error, "engine_AnimatePositionByOffset callback failed: {0}", err.what());
				}
			});
			return 0;
		});

		state["engine_GetLocalPlayer_PlayerIndex"] = LuaFunction([&](Nz::UInt8 localIndex) -> Nz::UInt16
		{
			return m_localPlayers[localIndex].playerIndex;
		});

		state["engine_GetLocalPlayerCount"] = LuaFunction([&]() -> std::size_t
		{
			return m_localPlayers.size();
		});

		state["engine_GetPlayerPosition"] = LuaFunction([&](sol::this_state L, Nz::UInt8 localIndex) -> sol::object
		{
			if (localIndex >= m_localPlayers.size())
				TriggerLuaArgError(L, 1, "invalid player index");

			auto& playerData = m_localPlayers[localIndex];
			if (playerData.controlledEntity)
				return sol::make_object(L, playerData.controlledEntity->GetPosition());
			else
				return sol::nil;
		});

		state["engine_GetActiveLayer"] = LuaFunction([&]()
		{
			return m_activeLayerIndex;
		});

		state["engine_OverridePlayerInputController"] = LuaFunction([&](sol::this_state L, Nz::UInt8 localIndex, std::shared_ptr<InputController> inputController)
		{
			if (localIndex >= m_localPlayers.size())
				TriggerLuaArgError(L, 1, "invalid player index");

			if (!inputController)
				TriggerLuaArgError(L, 2, "invalid input controller");

			m_localPlayers[localIndex].inputController = std::move(inputController);
		});

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
			m_gamemode = std::make_shared<ClientGamemode>(*this, m_scriptingContext, m_gamemodeName, m_gamemodeProperties);
			m_gamemode->ExecuteCallback<GamemodeEvent::Init>();
		}
		else
			m_gamemode->Reload();
	}

	void LocalMatch::RegisterEntity(EntityId uniqueId, LocalLayerEntityHandle entity)
	{
		assert(m_entitiesByUniqueId.find(uniqueId) == m_entitiesByUniqueId.end());
		m_entitiesByUniqueId.emplace(uniqueId, std::move(entity));
	}

	const Ndk::EntityHandle& LocalMatch::RetrieveEntityByUniqueId(EntityId uniqueId) const
	{
		auto it = m_entitiesByUniqueId.find(uniqueId);
		if (it == m_entitiesByUniqueId.end())
			return Ndk::EntityHandle::InvalidHandle;

		return it.value()->GetEntity();
	}

	EntityId LocalMatch::RetrieveUniqueIdByEntity(const Ndk::EntityHandle& entity) const
	{
		if (!entity || !entity->HasComponent<LocalMatchComponent>())
			return InvalidEntityId;

		return entity->GetComponent<LocalMatchComponent>().GetUniqueId();
	}

	void LocalMatch::UnregisterEntity(EntityId uniqueId)
	{
		auto it = m_entitiesByUniqueId.find(uniqueId);
		assert(it != m_entitiesByUniqueId.end());
		m_entitiesByUniqueId.erase(it);
	}

	bool LocalMatch::Update(float elapsedTime)
	{
		if (m_isLeavingMatch)
			return false;

		if (m_scriptingContext)
			m_scriptingContext->Update();

		SharedMatch::Update(elapsedTime);

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
							CompressedUnsigned<Nz::UInt16> layerId;
							CompressedUnsigned<Nz::UInt32> entityId;
							debugPacket >> layerId;
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

							auto& layer = m_layers[layerId];
							if (layer->IsEnabled())
							{
								if (auto entityOpt = layer->GetEntityByServerId(entityId))
								{
									LocalLayerEntity& entity = entityOpt.value();
									LocalLayerEntity* ghostEntity = entity.GetGhost();
									/*if (isPhysical)
										ghostEntity->UpdateState(position, rotation, linearVelocity, angularVelocity);
									else*/
										ghostEntity->UpdateState(position, rotation);

									ghostEntity->SyncVisuals();
								}
							}

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

		for (auto& layer : m_layers)
		{
			if (layer->IsEnabled())
				layer->PreFrameUpdate(elapsedTime);
		}

		if (m_gamemode)
			m_gamemode->ExecuteCallback<GamemodeEvent::Frame>(elapsedTime);

		for (auto& layer : m_layers)
		{
			if (layer->IsEnabled())
				layer->FrameUpdate(elapsedTime);
		}

		m_animationManager.Update(elapsedTime);

		for (auto& layerPtr : m_layers)
		{
			if (layerPtr->IsEnabled())
				layerPtr->SyncVisuals();
		}

		m_renderWorld.Update(elapsedTime);

		if (m_gamemode)
			m_gamemode->ExecuteCallback<GamemodeEvent::PostFrame>(elapsedTime);

		for (auto& layer : m_layers)
		{
			if (layer->IsEnabled())
				layer->PostFrameUpdate(elapsedTime);
		}

		/*Ndk::PhysicsSystem2D::DebugDrawOptions options;
		options.polygonCallback = [](const Nz::Vector2f* vertices, std::size_t vertexCount, float radius, Nz::Color outline, Nz::Color fillColor, void* userData)
		{
			//Nz::DebugDrawer::SetPrimaryColor(outline);
			for (std::size_t i = 0; i < vertexCount - 1; ++i)
				Nz::DebugDrawer::DrawLine(vertices[i], vertices[i + 1]);

			Nz::DebugDrawer::DrawLine(vertices[vertexCount - 1], vertices[0]);
		};

		options.segmentCallback = [](const Nz::Vector2f& first, const Nz::Vector2f& second, Nz::Color color, void* userdata)
		{
			//Nz::DebugDrawer::SetPrimaryColor(color);
			Nz::DebugDrawer::DrawLine(first, second);
		};

		options.thickSegmentCallback = [](const Nz::Vector2f& first, const Nz::Vector2f& second, float thickness, Nz::Color outline, Nz::Color fillColor, void* userdata)
		{
			//Nz::DebugDrawer::SetPrimaryColor(outline);
			Nz::DebugDrawer::DrawLine(first, second);
		};

		m_layers[0]->GetWorld().GetSystem<Ndk::PhysicsSystem2D>().DebugDraw(options);*/

		return true;
	}
	
	void LocalMatch::BindEscapeMenu()
	{
		m_escapeMenu.OnLeaveMatch.Connect([this](EscapeMenu*)
		{
			Quit();
		});

		m_escapeMenu.OnQuitApp.Connect([this](EscapeMenu*)
		{
			m_application.Quit();
		});
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

		m_session.OnEntitiesScale.Connect([this](ClientSession* /*session*/, const Packets::EntitiesScale& scale)
		{
			PushTickPacket(scale.stateTick, scale);
		});

		m_session.OnEntityPhysics.Connect([this](ClientSession* /*session*/, const Packets::EntityPhysics& physics)
		{
			PushTickPacket(physics.stateTick, physics);
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
		
		m_session.OnPlayerJoined.Connect([this](ClientSession* /*session*/, const Packets::PlayerJoined& playerJoined)
		{
			HandlePlayerJoined(playerJoined);
		});
		
		m_session.OnPlayerLeaving.Connect([this](ClientSession* /*session*/, const Packets::PlayerLeaving& playerLeaving)
		{
			HandlePlayerLeaving(playerLeaving);
		});
		
		m_session.OnPlayerNameUpdate.Connect([this](ClientSession* /*session*/, const Packets::PlayerNameUpdate& playerNameUpdate)
		{
			HandlePlayerNameUpdate(playerNameUpdate);
		});

		m_session.OnPlayerPingUpdate.Connect([this](ClientSession* /*session*/, const Packets::PlayerPingUpdate& playerPingUpdate)
		{
			HandlePlayerPingUpdate(playerPingUpdate);
		});

		m_session.OnPlayerWeapons.Connect([this](ClientSession* /*session*/, const Packets::PlayerWeapons& weapons)
		{
			PushTickPacket(weapons.stateTick, weapons);
		});

		m_session.OnScriptPacket.Connect([this](ClientSession* /*session*/, const Packets::ScriptPacket& scriptPacket)
		{
			HandleScriptPacket(scriptPacket);
		});
	}

	void LocalMatch::BindSignals(ClientEditorApp& burgApp, Nz::RenderWindow* window, Ndk::Canvas* canvas)
	{
		m_chatBox.OnChatMessage.Connect([this](const std::string& message)
		{
			Packets::PlayerChat chatPacket;
			chatPacket.localIndex = 0;
			chatPacket.message = message;

			m_session.SendPacket(chatPacket);
		});

		m_onGainedFocus.Connect(window->GetEventHandler().OnGainedFocus, [this](const Nz::EventHandler* /*eventHandler*/)
		{
			m_hasFocus = true;
		});

		m_onLostFocus.Connect(window->GetEventHandler().OnLostFocus, [this](const Nz::EventHandler* /*eventHandler*/)
		{
			m_hasFocus = false;
		});

		m_onUnhandledKeyPressed.Connect(canvas->OnUnhandledKeyPressed, [this](const Nz::EventHandler*, const Nz::WindowEvent::KeyEvent& event)
		{
			switch (event.virtualKey)
			{
				case Nz::Keyboard::VKey::Escape:
				{
					if (m_escapeMenu.IsVisible())
						m_escapeMenu.Hide();
					else if (m_chatBox.IsOpen())
						m_chatBox.Close();
					else
						m_escapeMenu.Show();

					break;
				}

				case Nz::Keyboard::VKey::F9:
					if (m_remoteConsole)
						m_remoteConsole->Hide();

					if (m_localConsole)
						m_localConsole->Show(!m_localConsole->IsVisible());

					break;

				case Nz::Keyboard::VKey::F10:
					if (m_localConsole)
						m_localConsole->Hide();

					if (m_remoteConsole)
						m_remoteConsole->Show(!m_remoteConsole->IsVisible());

					break;

				case Nz::Keyboard::VKey::Return:
				{
					if (m_chatBox.IsOpen())
					{
						m_chatBox.SendMessage();
						m_chatBox.Close();
					}
					else
						m_chatBox.Open();
				
					break;
				}

				case Nz::Keyboard::VKey::Tab:
				{
					if (!m_scoreboard)
						InitializeScoreboard();
					else
						m_scoreboard->Show(true);

					break;
				}

				default:
					break;
			}
		});

		m_onUnhandledKeyReleased.Connect(canvas->OnUnhandledKeyReleased, [this](const Nz::EventHandler*, const Nz::WindowEvent::KeyEvent& event)
		{
			switch (event.virtualKey)
			{
				case Nz::Keyboard::VKey::Tab:
					if (m_scoreboard)
						m_scoreboard->Hide();

					break;

				default:
					break;
			};
		});

		m_onRenderTargetSizeChange.Connect(window->OnRenderTargetSizeChange, [this](const Nz::RenderTarget* renderTarget)
		{
			Nz::Vector2f size = Nz::Vector2f(renderTarget->GetSize());

			if (m_scoreboard)
			{
				m_scoreboard->Resize({ size.x * 0.75f, size.y * 0.75f });
				m_scoreboard->Center();
			}
		});

		m_nicknameUpdateSlot.Connect(burgApp.GetPlayerSettings().GetStringUpdateSignal("Player.Name"), [this](const std::string& newValue)
		{
			Packets::UpdatePlayerName nameUpdate;
			nameUpdate.newName = newValue;
			nameUpdate.localIndex = 0; //< FIXME

			m_session.SendPacket(nameUpdate);
		});
	}

	Nz::UInt64 LocalMatch::EstimateServerTick() const
	{
		return GetCurrentTick() - m_averageTickError.GetAverageValue();
	}

	void LocalMatch::HandleChatMessage(const Packets::ChatMessage& packet)
	{
		//TODO: Implement this in gamemode callback
		if (packet.playerIndex == Packets::ChatMessage::InvalidPlayer)
		{
			m_chatBox.PrintMessage({ 
				Chatbox::ColorItem { Nz::Color(173, 216, 230) }, // light blue
				Chatbox::TextItem { packet.content }
			});
		}
		else
		{
			if (packet.playerIndex >= m_matchPlayers.size())
				return;

			std::string playerName;
			if (const auto& playerOpt = m_matchPlayers[packet.playerIndex])
				playerName = playerOpt->GetName();
			else
				playerName = "<disconnected>";

			m_chatBox.PrintMessage({ 
				Chatbox::ColorItem { Nz::Color::Yellow },
				Chatbox::TextItem { std::move(playerName) },
				Chatbox::ColorItem { Nz::Color::White },
				Chatbox::TextItem { ": " },
				Chatbox::TextItem { packet.content }
			});
		}
	}

	void LocalMatch::HandleConsoleAnswer(const Packets::ConsoleAnswer& packet)
	{
		if (m_remoteConsole)
			m_remoteConsole->Print(packet.response, packet.color);
	}

	void LocalMatch::HandlePlayerJoined(const Packets::PlayerJoined& packet)
	{
		if (packet.playerIndex >= m_matchPlayers.size())
			m_matchPlayers.resize(packet.playerIndex + 1);

		LocalPlayer& newPlayer = m_matchPlayers[packet.playerIndex].emplace(packet.playerIndex, packet.playerName);

		m_gamemode->ExecuteCallback<GamemodeEvent::PlayerJoined>(newPlayer.CreateHandle());
	}

	void LocalMatch::HandlePlayerLeaving(const Packets::PlayerLeaving& packet)
	{
		if (packet.playerIndex >= m_matchPlayers.size())
			return;

		auto& playerOpt = m_matchPlayers[packet.playerIndex];
		if (!playerOpt)
			return;

		m_gamemode->ExecuteCallback<GamemodeEvent::PlayerLeave>(playerOpt->CreateHandle());

		playerOpt.reset();
	}

	void LocalMatch::HandlePlayerNameUpdate(const Packets::PlayerNameUpdate& packet)
	{
		if (packet.playerIndex >= m_matchPlayers.size())
			return;

		auto& playerOpt = m_matchPlayers[packet.playerIndex];
		if (!playerOpt)
			return;

		m_gamemode->ExecuteCallback<GamemodeEvent::PlayerNameUpdate>(playerOpt->CreateHandle(), packet.newName);
		playerOpt->UpdateName(packet.newName);
	}

	void LocalMatch::HandlePlayerPingUpdate(const Packets::PlayerPingUpdate& packet)
	{
		for (const auto& playerData : packet.players)
		{
			if (playerData.playerIndex >= m_matchPlayers.size() || !m_matchPlayers[playerData.playerIndex])
				continue;

			m_matchPlayers[playerData.playerIndex]->UpdatePing(playerData.ping);
		}

		m_gamemode->ExecuteCallback<GamemodeEvent::PlayerPingUpdate>();
	}

	void LocalMatch::HandleScriptPacket(const Packets::ScriptPacket& packet)
	{
		const ScriptHandlerRegistry& registry = GetScriptPacketHandlerRegistry();
		const NetworkStringStore& stringStore = GetNetworkStringStore();

		const std::string& packetName = stringStore.GetString(packet.nameIndex);

		registry.Call(packetName, IncomingNetworkPacket(stringStore, packet));
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

		auto layerEntityOpt = layerPtr->GetEntityByServerId(packet.entityId);
		if (!layerEntityOpt)
			return;

		LocalLayerEntity& layerEntity = layerEntityOpt.value();

		if (m_localPlayers[packet.localIndex].controlledEntity)
		{
			auto& controlledEntity = m_localPlayers[packet.localIndex].controlledEntity;
			controlledEntity->GetEntity()->RemoveComponent<Ndk::ListenerComponent>();

			m_layers[controlledEntity->GetLayerIndex()]->EnablePrediction(false);
		}

		m_localPlayers[packet.localIndex].controlledEntity = layerEntity.CreateHandle();
		m_localPlayers[packet.localIndex].controlledEntity->GetEntity()->AddComponent<Ndk::ListenerComponent>();

		// Ensure prediction is enabled on all player-controlled layers
		for (auto& playerData : m_localPlayers)
		{
			if (!playerData.controlledEntity)
				continue;

			LayerIndex layerIndex = playerData.controlledEntity->GetLayerIndex();
			m_layers[layerIndex]->EnablePrediction();
		}
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

	void LocalMatch::HandleTickPacket(Packets::EntitiesScale&& packet)
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

	void LocalMatch::HandleTickPacket(Packets::EntityPhysics&& packet)
	{
		assert(packet.entityId.layerId < m_layers.size());
		auto& layer = m_layers[packet.entityId.layerId];
		layer->HandlePacket(packet);
	}

	void LocalMatch::HandleTickPacket(Packets::EntityWeapon&& packet)
	{
		assert(packet.entityId.layerId < m_layers.size());
		auto& layer = m_layers[packet.entityId.layerId];
		layer->HandlePacket(packet);
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
		if (Nz::Keyboard::IsKeyPressed(Nz::Keyboard::Scancode::Q))
			return;

		m_inactiveEntities.clear();

		auto inputIt = std::find_if(m_predictedInputs.begin(), m_predictedInputs.end(), [lastInputTick = packet.lastInputTick](const PredictedInput& input)
		{
			return input.inputTick == lastInputTick;
		});
		if (inputIt != m_predictedInputs.end())
		{
			bool performReconciliation = [&]
			{
				// Check if reconciliation is required (were all packets entities at the same position back then?)
				std::size_t offset = 0;
				for (auto&& packetLayer : packet.layers)
				{
					assert(packetLayer.layerIndex < m_layers.size());
					auto& layer = m_layers[packetLayer.layerIndex];
					if (!layer->IsEnabled() || !layer->IsPredictionEnabled())
						continue;

					auto layerIt = std::find_if(inputIt->layers.begin(), inputIt->layers.end(), [&](const auto& layer) { return layer.layerIndex == packetLayer.layerIndex; });
					if (layerIt == inputIt->layers.end())
						continue;

					const auto& layerData = *layerIt;

					for (std::size_t i = 0; i < packetLayer.entityCount; ++i)
					{
						auto& packetEntity = packet.entities[offset + i];
						EntityId uniqueId = layer->GetUniqueIdByServerId(packetEntity.id);
						if (uniqueId == 0)
							continue;

						auto it = layerData.entities.find(uniqueId);
						if (it == layerData.entities.end())
							continue;

						constexpr float MaxPositionError = 5.f; //< five pixels
						constexpr float MaxRotationError = Nz::DegreeToRadian(5.f);

						auto& entityData = it.value();
						if (!CompareWithEpsilon(entityData.position, packetEntity.position, MaxPositionError) ||
							!CompareWithEpsilon(entityData.rotation, packetEntity.rotation, MaxRotationError))
						{
							/*Nz::Vector2f posDiff = entityData.position - packetEntity.position;
							Nz::RadianAnglef rotDiff = entityData.rotation - packetEntity.rotation;

							bwLog(GetLogger(), LogLevel::Debug, "Prediction error for entity #{} (position diff: {}, rotation diff: {})", uniqueId, posDiff.ToString().ToStdString(), rotDiff.ToString().ToStdString());*/
							return true;
						}
					}

					offset += packetLayer.entityCount;
				}

				return false;
			}();

			if (!performReconciliation)
				return;

			//bwLog(GetLogger(), LogLevel::Debug, "Too much error detected, performing reconciliation...");

			for (const auto& layerData : inputIt->layers)
			{
				assert(layerData.layerIndex < m_layers.size());
				auto& layer = m_layers[layerData.layerIndex];
				if (!layer->IsEnabled() || !layer->IsPredictionEnabled())
					continue;

				layer->ForEachLayerEntity([&](LocalLayerEntity& layerEntity)
				{
					EntityId uniqueId = layerEntity.GetUniqueId();
					auto it = layerData.entities.find(uniqueId);
					if (it != layerData.entities.end())
					{
						auto& entityData = it.value();
						if (entityData.isPhysical)
							layerEntity.UpdateState(entityData.position, entityData.rotation, entityData.linearVelocity, entityData.angularVelocity);
						else
							layerEntity.UpdateState(entityData.position, entityData.rotation);
					}
					else if (layerEntity.IsEnabled())
					{
						layerEntity.Disable();
						m_inactiveEntities.insert(uniqueId);
					}
				});
			}
		}

		// Apply physics state to all layers
		std::size_t offset = 0;
		for (auto&& packetLayer : packet.layers)
		{
			assert(packetLayer.layerIndex < m_layers.size());
			auto& layer = m_layers[packetLayer.layerIndex];

			for (std::size_t i = 0; i < packetLayer.entityCount; ++i)
			{
				auto& packetEntity = packet.entities[offset + i];

				auto entityOpt = layer->GetEntityByServerId(packetEntity.id);
				if (!entityOpt)
					continue;

				LocalLayerEntity& localEntity = entityOpt.value();
				if (localEntity.IsPhysical())
				{
					if (packetEntity.physicsProperties.has_value())
					{
						auto& physData = packetEntity.physicsProperties.value();
						localEntity.UpdateState(packetEntity.position, packetEntity.rotation, physData.linearVelocity, physData.angularVelocity);
					}
					else
					{
						bwLog(GetLogger(), LogLevel::Warning, "Entity {} has client-side physics but server sends no data", localEntity.GetUniqueId());
						localEntity.UpdateState(packetEntity.position, packetEntity.rotation);
					}
				}
				else
				{
					if (packetEntity.physicsProperties.has_value())
						bwLog(GetLogger(), LogLevel::Warning, "Received physics properties for entity {} which is not physical client-side", localEntity.GetUniqueId());

					localEntity.UpdateState(packetEntity.position, packetEntity.rotation);
				}

				if (packetEntity.playerMovement)
					localEntity.UpdatePlayerMovement(packetEntity.playerMovement->isFacingRight);
			}

			offset += packetLayer.entityCount;
		}

		// Remove treated inputs
		auto firstClientInput = std::find_if(m_predictedInputs.begin(), m_predictedInputs.end(), [lastInputTick = packet.lastInputTick](const PredictedInput& input)
		{
			return IsMoreRecent(input.inputTick, lastInputTick);
		});
		m_predictedInputs.erase(m_predictedInputs.begin(), firstClientInput);

		for (const PredictedInput& input : m_predictedInputs)
		{
			for (std::size_t i = 0; i < m_localPlayers.size(); ++i)
			{
				auto& controllerData = m_localPlayers[i];
				if (controllerData.controlledEntity)
				{
					auto& controlledEntity = controllerData.controlledEntity->GetEntity();

					InputComponent& entityInputs = controlledEntity->GetComponent<InputComponent>();
					const auto& playerInputData = input.inputs[i];
					entityInputs.UpdateInputs(playerInputData.input);
					entityInputs.UpdatePreviousInputs(playerInputData.previousInput);

					if (playerInputData.movement)
					{
						auto& playerMovement = controlledEntity->GetComponent<PlayerMovementComponent>();
						auto& playerPhysics = controlledEntity->GetComponent<Ndk::PhysicsComponent2D>();
						playerMovement.UpdateGroundState(playerInputData.movement->isOnGround);
						playerMovement.UpdateJumpTime(playerInputData.movement->jumpTime);
						playerMovement.UpdateWasJumpingState(playerInputData.movement->wasJumping);

						playerPhysics.SetFriction(0, playerInputData.movement->friction);
						playerPhysics.SetSurfaceVelocity(0, playerInputData.movement->surfaceVelocity);
					}

					for (auto&& weaponData : playerInputData.weapons)
					{
						if (!weaponData.entity)
							continue;

						weaponData.entity->GetComponent<WeaponComponent>().SetAttacking(weaponData.isAttacking);
					}
				}
			}

			for (auto& layer : m_layers)
			{
				if (layer->IsEnabled() && layer->IsPredictionEnabled())
					layer->TickUpdate(GetTickDuration());
			}

			for (auto it = m_inactiveEntities.begin(); it != m_inactiveEntities.end();)
			{
				EntityId uniqueId = *it;
				auto FindAndUpdate = [&]
				{
					for (const auto& layerData : input.layers)
					{
						assert(layerData.layerIndex < m_layers.size());
						auto& layer = m_layers[layerData.layerIndex];
						if (!layer->IsEnabled() || !layer->IsPredictionEnabled())
							continue;

						auto entityIt = layerData.entities.find(uniqueId);
						if (entityIt != layerData.entities.end())
						{
							auto layerEntityOpt = layer->GetEntity(uniqueId);
							assert(layerEntityOpt);
							LocalLayerEntity& layerEntity = layerEntityOpt.value();
							layerEntity.Enable();

							auto& entityData = entityIt.value();
							if (entityData.isPhysical)
								layerEntity.UpdateState(entityData.position, entityData.rotation, entityData.linearVelocity, entityData.angularVelocity);
							else
								layerEntity.UpdateState(entityData.position, entityData.rotation);

							return true;
						}
					}

					return false;
				};

				if (FindAndUpdate())
					it = m_inactiveEntities.erase(it);
				else
					++it;
			}
		}

		// Prevent locking entities forever
		for (EntityId uniqueId : m_inactiveEntities)
		{
			for (auto& layer : m_layers)
			{
				if (!layer->IsEnabled() || !layer->IsPredictionEnabled())
					continue;

				auto entityOpt = layer->GetEntity(uniqueId);
				if (!entityOpt)
					continue;

				LocalLayerEntity& entity = entityOpt.value();
				entity.Enable();
				break;
			}
		}
	}

	void LocalMatch::HandleTickPacket(Packets::PlayerLayer&& packet)
	{
		m_localPlayers[packet.localIndex].layerIndex = packet.layerIndex;

		m_gamemode->ExecuteCallback<GamemodeEvent::ChangeLayer>(m_activeLayerIndex, static_cast<LayerIndex>(packet.layerIndex));

		m_activeLayerIndex = packet.layerIndex;

		auto& layer = m_layers[m_activeLayerIndex];

		m_colorBackground->SetColor(layer->GetBackgroundColor());
		auto& visibleLayer = m_currentLayer->GetComponent<VisibleLayerComponent>();
		visibleLayer.Clear();
		visibleLayer.RegisterVisibleLayer(*layer, 0, Nz::Vector2f::Unit(), Nz::Vector2f::Unit());
	}

	void LocalMatch::HandleTickPacket(Packets::PlayerWeapons&& packet)
	{
		auto& playerData = m_localPlayers[packet.localIndex];
		playerData.weapons.clear();

		assert(packet.layerIndex < m_layers.size());
		auto& layer = m_layers[packet.layerIndex];

		for (auto weaponEntityIndex : packet.weaponEntities)
		{
			auto entityOpt = layer->GetEntityByServerId(weaponEntityIndex);
			if (!entityOpt)
			{
				bwLog(GetLogger(), LogLevel::Warning, "Local player #{0} weapon entity {1} doesn't exist", +packet.localIndex, weaponEntityIndex);
				continue;
			}

			LocalLayerEntity& layerEntity = entityOpt.value();

			assert(layerEntity.GetEntity()->HasComponent<WeaponComponent>());

			playerData.weapons.emplace_back(layerEntity.GetEntity());

			auto& scriptComponent = layerEntity.GetEntity()->GetComponent<ScriptComponent>();
			bwLog(GetLogger(), LogLevel::Info, "Local player #{0} has weapon {1}", +packet.localIndex, scriptComponent.GetElement()->fullName);
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

				//bwLog(GetLogger(), LogLevel::Debug, "Error: {}", tickError);
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
		m_remoteConsole.emplace(m_renderTarget, m_canvas);
		m_remoteConsole->SetExecuteCallback([this](const std::string& command) -> bool
		{
			Packets::PlayerConsoleCommand commandPacket;
			commandPacket.command = command;
			commandPacket.localIndex = 0;

			m_session.SendPacket(commandPacket);

			return true;
		});
	}

	void LocalMatch::InitializeScoreboard()
	{
		m_scoreboard = m_canvas->Add<Scoreboard>(GetLogger());
		m_gamemode->ExecuteCallback<GamemodeEvent::InitScoreboard>(m_scoreboard->CreateHandle());

		Nz::Vector2f size = Nz::Vector2f(m_renderTarget->GetSize());

		m_scoreboard->Resize({ size.x * 0.75f, size.y * 0.75f });
		m_scoreboard->Center();
	}

	void LocalMatch::OnTick(bool lastTick)
	{
		Nz::UInt16 estimatedServerTick = GetNetworkTick(EstimateServerTick());

		Nz::UInt16 handledTick = AdjustServerTick(estimatedServerTick); //< To handle network jitter

		//bwLog(GetLogger(), LogLevel::Debug, "Executing packets for tick {}", handledTick);

		auto it = m_tickedPackets.begin();
		while (it != m_tickedPackets.end() && (it->serverTick == handledTick || IsMoreRecent(handledTick, it->serverTick)))
		{
			HandleTickPacket(std::move(it->content));
			++it;
		}
		m_tickedPackets.erase(m_tickedPackets.begin(), it);

		if (lastTick)
		{
			SendInputs(estimatedServerTick, true);

			for (std::size_t i = 0; i < m_localPlayers.size(); ++i)
			{
				auto& controllerData = m_localPlayers[i];
				if (controllerData.controlledEntity)
				{
					auto& entity = controllerData.controlledEntity->GetEntity();
					if (entity->HasComponent<InputComponent>())
					{
						auto& entityInputs = entity->GetComponent<InputComponent>();
						entityInputs.UpdateInputs(controllerData.lastInputData);
					}
				}
			}
		}

		if (m_gamemode)
			m_gamemode->ExecuteCallback<GamemodeEvent::Tick>();

		for (auto& layer : m_layers)
		{
			if (layer->IsEnabled())
				layer->TickUpdate(GetTickDuration());
		}

		if (lastTick)
		{
			// Remember predicted ticks for improving over time
			if (m_tickPredictions.size() >= static_cast<std::size_t>(std::ceil(2 / GetTickDuration()))) //< Remember at most 2s of inputs
				m_tickPredictions.erase(m_tickPredictions.begin());

			auto& prediction = m_tickPredictions.emplace_back();
			prediction.serverTick = estimatedServerTick;
			prediction.tickError = m_averageTickError.GetAverageValue();

			// Remember inputs for reconciliation
			PredictedInput& predictedInputs = m_predictedInputs.emplace_back();
			predictedInputs.inputTick = GetNetworkTick();
			//predictedInputs.inputTick = GetNetworkTick();

			predictedInputs.inputs.resize(m_localPlayers.size());
			for (std::size_t i = 0; i < m_localPlayers.size(); ++i)
			{
				auto& controllerData = m_localPlayers[i];

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

						movementData.friction = playerPhysics.GetFriction(0);
						movementData.surfaceVelocity = playerPhysics.GetSurfaceVelocity(0);
					}

					if (entity->HasComponent<InputComponent>())
					{
						auto& entityInputs = entity->GetComponent<InputComponent>();
						entityInputs.UpdateInputs(playerData.input);
						playerData.previousInput = entityInputs.GetPreviousInputs();
					}
				}

				for (auto&& weaponEntity : controllerData.weapons)
				{
					if (!weaponEntity || !weaponEntity->HasComponent<WeaponComponent>())
						continue;

					auto& weaponData = playerData.weapons.emplace_back();
					weaponData.entity = weaponEntity;
					weaponData.isAttacking = weaponEntity->GetComponent<WeaponComponent>().IsAttacking();
				}
			}

			for (auto& layer : m_layers)
			{
				if (layer->IsEnabled())
				{
					auto& layerData = predictedInputs.layers.emplace_back();
					layerData.layerIndex = layer->GetLayerIndex();

					layer->ForEachLayerEntity([&](LocalLayerEntity& layerEntity)
					{
						assert(layerData.entities.find(layerEntity.GetUniqueId()) == layerData.entities.end());
						auto& entityData = layerData.entities.emplace(layerEntity.GetUniqueId(), PredictedInput::EntityData{}).first.value();
						entityData.isPhysical = layerEntity.IsPhysical();

						if (entityData.isPhysical)
						{
							entityData.position = layerEntity.GetPhysicalPosition();
							entityData.rotation = layerEntity.GetPhysicalRotation();

							entityData.angularVelocity = layerEntity.GetAngularVelocity();
							entityData.linearVelocity = layerEntity.GetLinearVelocity();
						}
						else
						{
							entityData.position = layerEntity.GetPosition();
							entityData.rotation = layerEntity.GetRotation();
						}
					});
				}
			}
		}
	}

	void LocalMatch::PushTickPacket(Nz::UInt16 tick, const TickPacketContent& packet)
	{
		//bwLog(GetLogger(), LogLevel::Debug, "Execute server tick in {}", tick - m_expectedServerTick.value());

		TickPacket newPacket;
		newPacket.serverTick = tick;
		newPacket.content = packet;

		//bwLog(GetLogger(), LogLevel::Debug, "Received packet of tick #{}", tick);

		auto it = std::upper_bound(m_tickedPackets.begin(), m_tickedPackets.end(), newPacket, [](const TickPacket& a, const TickPacket& b)
		{
			return IsMoreRecent(b.serverTick, a.serverTick);
		});

		m_tickedPackets.emplace(it, std::move(newPacket));
	}

	bool LocalMatch::SendInputs(Nz::UInt16 serverTick, bool force)
	{
		assert(m_localPlayers.size() == m_inputPacket.inputs.size());

		m_inputPacket.estimatedServerTick = serverTick;
		m_inputPacket.inputTick = GetNetworkTick();

		//bwLog(GetLogger(), LogLevel::Debug, "Send input tick: {}", m_inputPacket.inputTick);

		bool checkInputs = m_hasFocus &&
		                   !m_chatBox.IsTyping() &&
		                  (!m_localConsole || !m_localConsole->IsVisible()) &&
		                  (!m_remoteConsole || !m_remoteConsole->IsVisible());

		bool hasInputData = false;
		for (std::size_t i = 0; i < m_localPlayers.size(); ++i)
		{
			auto& controllerData = m_localPlayers[i];
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
