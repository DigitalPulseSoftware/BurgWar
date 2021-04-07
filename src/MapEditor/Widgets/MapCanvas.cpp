// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Widgets/MapCanvas.hpp>
#include <CoreLib/LogSystem/Logger.hpp>
#include <ClientLib/Components/VisibleLayerComponent.hpp>
#include <ClientLib/Scripting/ClientEditorScriptingLibrary.hpp>
#include <ClientLib/Scripting/ClientElementLibrary.hpp>
#include <ClientLib/Scripting/ClientWeaponLibrary.hpp>
#include <MapEditor/Gizmos/PositionGizmo.hpp>
#include <MapEditor/Components/CanvasComponent.hpp>
#include <MapEditor/Scripting/EditorElementLibrary.hpp>
#include <MapEditor/Scripting/EditorEntityLibrary.hpp>
#include <MapEditor/Scripting/EditorGamemode.hpp>
#include <MapEditor/Scripting/EditorScriptedEntity.hpp>
#include <MapEditor/Scripting/EditorScriptingLibrary.hpp>
#include <MapEditor/Widgets/EditorWindow.hpp>
#include <Nazara/Graphics/ColorBackground.hpp>
#include <Nazara/Graphics/Sprite.hpp>
#include <Nazara/Math/Ray.hpp>
#include <NDK/Components/CameraComponent.hpp>
#include <NDK/Components/GraphicsComponent.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <NDK/Systems/ListenerSystem.hpp>
#include <NDK/Systems/PhysicsSystem2D.hpp>
#include <NDK/Systems/RenderSystem.hpp>

namespace bw
{
	MapCanvas::MapCanvas(EditorWindow& editor, QWidget* parent) :
	SharedMatch(editor, LogSide::Editor, "editor", 1.f / 60.f),
	WorldCanvas(parent),
	m_editor(editor)
	{
		Ndk::World& world = GetWorld();
		world.AddSystem<Ndk::ListenerSystem>();
		world.AddSystem<Ndk::PhysicsSystem2D>();

		EnableCameraControl(true);

		const CameraMovement& cameraController = GetCameraController();
		cameraController.OnCameraMoved.Connect([this](CameraMovement* /*controller*/)
		{
			UpdateGrid();
		});

		cameraController.OnCameraZoomUpdated.Connect([this](CameraMovement* controller)
		{
			OnCameraZoomFactorUpdated(this, controller->ComputeZoomFactor());
			UpdateGrid();
		});

		m_currentLayerEntity = GetWorld().CreateEntity();
		m_currentLayerEntity->AddComponent<Ndk::NodeComponent>();
		m_currentLayerEntity->AddComponent<VisibleLayerComponent>(GetWorld());

		const ConfigFile& config = editor.GetConfig();
		const std::string& assetDir = config.GetStringValue("Resources.AssetDirectory");
		const std::string& scriptDir = config.GetStringValue("Resources.ScriptDirectory");

		m_assetDirectory = std::make_shared<VirtualDirectory>(assetDir);
		m_scriptDirectory = std::make_shared<VirtualDirectory>(scriptDir);

		m_assetStore.emplace(GetLogger(), m_assetDirectory);
		ReloadScripts();
	}

	MapCanvas::~MapCanvas()
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

	void MapCanvas::Clear()
	{
		m_currentLayer.reset();
		m_entitiesByUniqueId.clear();
		m_layers.clear();

		ClearEntitySelection(); //< Force disconnection because entity destruction does not occur until the world next update
	}

	void MapCanvas::ClearEntitySelection()
	{
		m_onGizmoEntityDestroyed.Disconnect();
		m_onLayerAlignmentUpdate.Disconnect(); //< Used by position gizmo
		m_entityGizmo.reset();
	}

	const Ndk::EntityHandle& MapCanvas::CreateEntity(LayerIndex layerIndex, EntityId uniqueId, const std::string& entityClass, const Nz::Vector2f& position, const Nz::DegreeAnglef& rotation, PropertyValueMap properties)
	{
		assert(layerIndex < m_layers.size());
		auto& layer = m_layers[layerIndex];
		return layer.CreateEntity(uniqueId, entityClass, position, rotation, properties).GetEntity();
	}

	void MapCanvas::DeleteEntity(EntityId entityId)
	{
		if (auto layerEntity = RetrieveLayerEntityByUniqueId(entityId))
		{
			LayerIndex layerIndex = layerEntity->GetLayerIndex();

			assert(layerIndex < m_layers.size());
			auto& layer = m_layers[layerIndex];
			layer.DeleteEntity(entityId);
		}
	}

	void MapCanvas::EditEntitiesPosition(const std::vector<EntityId>& entityIds)
	{
		const Map& mapData = m_editor.GetWorkingMap();
		const auto& currentLayerOpt = m_editor.GetCurrentLayer();
		assert(currentLayerOpt);

		const Map::Layer& layerData = mapData.GetLayer(*currentLayerOpt);

		std::vector<LayerVisualEntityHandle> entities;
		for (EntityId uniqueId : entityIds)
		{
			auto it = m_entitiesByUniqueId.find(uniqueId);
			if (it == m_entitiesByUniqueId.end())
				continue;

			entities.emplace_back(it->second);
		}

		if (entities.empty())
			return;

		std::unique_ptr<PositionGizmo> positionGizmo = std::make_unique<PositionGizmo>(GetCamera(), GetWorld(), std::move(entities), layerData.positionAlignment);
		positionGizmo->OnPositionUpdated.Connect([this](PositionGizmo* emitter, Nz::Vector2f offset)
		{
			std::vector<EntityId> ids = BuildEntityIds(emitter->GetTargetEntities());
			OnEntitiesPositionUpdated(this, ids.data(), ids.size(), offset);
		});

		m_onLayerAlignmentUpdate.Connect(m_editor.OnLayerAlignmentUpdate, [this, gizmo = positionGizmo.get()](EditorWindow* /*editor*/, LayerIndex layerIndex, const Nz::Vector2f& newAlignment)
		{
			auto currentLayerOpt = m_editor.GetCurrentLayer();
			if (!currentLayerOpt || *currentLayerOpt != layerIndex)
				return;

			gizmo->UpdatePositionAlignment(newAlignment);
		});

		m_entityGizmo = std::move(positionGizmo);
		/*m_onGizmoEntityDestroyed.Connect(entity->OnEntityDestruction, [this](Ndk::Entity* entity)
		{
			assert(m_entityGizmo->GetTargetEntity() == entity);
			NazaraUnused(entity);

			ClearEntitySelection();
		});*/
	}

	void MapCanvas::ForEachEntity(std::function<void(const Ndk::EntityHandle& entity)> func)
	{
		for (auto&& [uniqueId, visualEntityHandle] : m_entitiesByUniqueId)
			func(visualEntityHandle->GetEntity());
	}

	EditorEntityStore& MapCanvas::GetEntityStore()
	{
		return *m_entityStore;
	}

	const EditorEntityStore& MapCanvas::GetEntityStore() const
	{
		return *m_entityStore;
	}

	MapCanvasLayer& MapCanvas::GetLayer(LayerIndex layerIndex)
	{
		assert(layerIndex < m_layers.size());
		return m_layers[layerIndex];
	}

	const MapCanvasLayer& MapCanvas::GetLayer(LayerIndex layerIndex) const
	{
		assert(layerIndex < m_layers.size());
		return m_layers[layerIndex];
	}

	LayerIndex MapCanvas::GetLayerCount() const
	{
		return LayerIndex(m_layers.size());
	}

	const NetworkStringStore& MapCanvas::GetNetworkStringStore() const
	{
		throw std::runtime_error("map editor has no network string store");
	}

	std::shared_ptr<const SharedGamemode> MapCanvas::GetSharedGamemode() const
	{
		return m_gamemode;
	}

	ClientWeaponStore& MapCanvas::GetWeaponStore()
	{
		return *m_weaponStore;
	}

	const ClientWeaponStore& MapCanvas::GetWeaponStore() const
	{
		return *m_weaponStore;
	}

	void MapCanvas::ReloadScripts()
	{
		if (!m_scriptingContext)
		{
			m_scriptingContext = std::make_shared<ScriptingContext>(GetLogger(), m_scriptDirectory);
			m_scriptingContext->LoadLibrary(std::make_shared<EditorScriptingLibrary>(*this));
			m_scriptingContext->LoadLibrary(std::make_shared<ClientEditorScriptingLibrary>(GetLogger(), *m_assetStore));
		}
		else
			m_scriptingContext->ReloadLibraries();

		sol::state& lua = m_scriptingContext->GetLuaState();
		lua["Editor"] = &m_editor;

		lua["engine_GetActiveLayer"] = LuaFunction([&]()
		{
			return m_editor.GetCurrentLayer();
		});

		lua["engine_GetPlayerPosition"] = LuaFunction([&]()
		{
			QPoint mousePosition = mapFromGlobal(QCursor::pos());
			return GetCamera().Unproject(Nz::Vector2f(mousePosition.x(), mousePosition.y()));
		});

		if (!m_entityStore)
		{
			m_entityStore.emplace(*this, *m_assetStore, GetLogger(), m_scriptingContext);
			m_entityStore->LoadLibrary(std::make_shared<EditorElementLibrary>(GetLogger(), *m_assetStore));
			m_entityStore->LoadLibrary(std::make_shared<EditorEntityLibrary>(m_editor, GetLogger(), *m_assetStore));
		}
		else
		{
			m_entityStore->ClearElements();
			m_entityStore->ReloadLibraries();
		}

		m_entityStore->LoadDirectory("entities");
		m_entityStore->Resolve();

		if (!m_weaponStore)
		{
			m_weaponStore.emplace(*m_assetStore, GetLogger(), m_scriptingContext);
			m_weaponStore->LoadLibrary(std::make_shared<EditorElementLibrary>(GetLogger(), *m_assetStore));
			m_weaponStore->LoadLibrary(std::make_shared<ClientWeaponLibrary>(GetLogger(), *m_assetStore));
		}
		else
		{
			m_weaponStore->ClearElements();
			m_weaponStore->ReloadLibraries();
		}

		m_weaponStore->LoadDirectory("weapons");
		m_weaponStore->Resolve();

		ForEachEntity([this](const Ndk::EntityHandle& entity)
		{
			if (entity->HasComponent<ScriptComponent>())
				m_entityStore->UpdateEntityElement(entity);
		});

		if (!m_gamemode)
		{
			m_gamemode = std::make_shared<EditorGamemode>(*this, m_scriptingContext, PropertyValueMap{});
			m_gamemode->ExecuteCallback<GamemodeEvent::Init>();
		}
		else
			m_gamemode->Reload();
	}

	void MapCanvas::ResetLayers(std::size_t layerCount)
	{
		assert(m_layers.empty());
		m_layers.reserve(layerCount);
		for (std::size_t i = 0; i < layerCount; ++i)
			m_layers.emplace_back(*this, LayerIndex(i));
	}

	const Ndk::EntityHandle& MapCanvas::RetrieveEntityByUniqueId(EntityId uniqueId) const
	{
		auto it = m_entitiesByUniqueId.find(uniqueId);
		if (it == m_entitiesByUniqueId.end())
			return Ndk::EntityHandle::InvalidHandle;

		return it.value()->GetEntity();
	}

	const LayerVisualEntityHandle& MapCanvas::RetrieveLayerEntityByUniqueId(EntityId uniqueId) const
	{
		auto it = m_entitiesByUniqueId.find(uniqueId);
		if (it == m_entitiesByUniqueId.end())
			return LayerVisualEntityHandle::InvalidHandle;

		return it->second;
	}

	EntityId MapCanvas::RetrieveUniqueIdByEntity(const Ndk::EntityHandle& entity) const
	{
		if (!entity || !entity->HasComponent<CanvasComponent>())
			return InvalidEntityId;

		return entity->GetComponent<CanvasComponent>().GetUniqueId();
	}

	void MapCanvas::ShowGrid(bool show)
	{
		if (show)
		{
			if (!m_gridEntity)
			{
				m_gridEntity = GetWorld().CreateEntity();
				m_gridEntity->AddComponent<Ndk::GraphicsComponent>();
				m_gridEntity->AddComponent<Ndk::NodeComponent>();

				UpdateGrid();
			}
		}
		else
			m_gridEntity.Reset();
	}

	void MapCanvas::UpdateActiveLayer(std::optional<LayerIndex> layerIndex)
	{
		if (m_currentLayer != layerIndex)
		{
			auto& visibleLayer = m_currentLayerEntity->GetComponent<VisibleLayerComponent>();
			visibleLayer.Clear();

			if (layerIndex)
			{
				assert(layerIndex < m_layers.size());
				visibleLayer.RegisterVisibleLayer(GetCamera(), m_layers[*layerIndex], 0, Nz::Vector2f::Unit(), Nz::Vector2f::Unit());
			}

			m_gamemode->ExecuteCallback<GamemodeEvent::ChangeLayer>(m_currentLayer, layerIndex);
			m_currentLayer = layerIndex;
		}
	}

	void MapCanvas::UpdateEntityPositionAndRotation(EntityId entityId, const Nz::Vector2f& position, const Nz::DegreeAnglef& rotation)
	{
		auto it = m_entitiesByUniqueId.find(entityId);
		assert(it != m_entitiesByUniqueId.end());

		LayerVisualEntity& layerVisual = *it->second;

		auto& nodeComponent = layerVisual.GetEntity()->GetComponent<Ndk::NodeComponent>();
		nodeComponent.SetPosition(position);
		nodeComponent.SetRotation(rotation);

		layerVisual.SyncVisuals();

		// Refresh gizmo if an entity it uses has been updated
		if (m_entityGizmo)
		{
			const auto& targetEntities = m_entityGizmo->GetTargetEntities();

			auto it = std::find_if(targetEntities.begin(), targetEntities.end(), [&](const LayerVisualEntityHandle& visualEntity) 
			{ 
				return visualEntity->GetUniqueId() == entityId; 
			});

			if (it != targetEntities.end())
				m_entityGizmo->Refresh();
		}
	}

	void MapCanvas::OnKeyPressed(const Nz::WindowEvent::KeyEvent& key)
	{
		switch (key.virtualKey)
		{
			case Nz::Keyboard::VKey::Delete:
			{
				if (m_entityGizmo)
				{
					std::vector<EntityId> ids = BuildEntityIds(m_entityGizmo->GetTargetEntities());
					OnDeleteEntities(this, ids.data(), ids.size());
				}

				break;
			}

			case Nz::Keyboard::VKey::LShift:
			case Nz::Keyboard::VKey::RShift:
			{
				OnMultiSelectionStateUpdated(this, true);
				break;
			}

			default:
				break;
		}
	}

	void MapCanvas::OnKeyReleased(const Nz::WindowEvent::KeyEvent& key)
	{
		switch (key.virtualKey)
		{
			case Nz::Keyboard::VKey::LShift:
			case Nz::Keyboard::VKey::RShift:
			{
				OnMultiSelectionStateUpdated(this, false);
				break;
			}

			default:
				break;
		}
	}

	void MapCanvas::OnMouseButtonPressed(const Nz::WindowEvent::MouseButtonEvent& mouseButton)
	{
		WorldCanvas::OnMouseButtonPressed(mouseButton);

		if (m_entityGizmo)
		{
			if (m_entityGizmo->OnMouseButtonPressed(mouseButton))
				return;
		}

		OnCanvasMouseButtonPressed(this, mouseButton);
	}

	void MapCanvas::OnMouseButtonReleased(const Nz::WindowEvent::MouseButtonEvent& mouseButton)
	{
		WorldCanvas::OnMouseButtonReleased(mouseButton);

		if (m_entityGizmo)
		{
			if (m_entityGizmo->OnMouseButtonReleased(mouseButton))
				return;
		}

		OnCanvasMouseButtonReleased(this, mouseButton);
	}

	void MapCanvas::OnMouseEntered()
	{
		OnCanvasMouseEntered(this);
	}

	void MapCanvas::OnMouseLeft()
	{
		OnCanvasMouseLeft(this);
	}

	void MapCanvas::OnMouseMoved(const Nz::WindowEvent::MouseMoveEvent& mouseMoved)
	{
		WorldCanvas::OnMouseMoved(mouseMoved);

		if (m_entityGizmo)
		{
			if (m_entityGizmo->OnMouseMoved(mouseMoved))
				return;
		}

		OnCanvasMouseMoved(this, mouseMoved);
	}

	void MapCanvas::OnTick(bool /*lastTick*/)
	{
		if (m_gamemode)
			m_gamemode->ExecuteCallback<GamemodeEvent::Tick>();

		for (MapCanvasLayer& layer : m_layers)
			layer.TickUpdate(GetTickDuration());
	}

	void MapCanvas::OnUpdate(float elapsedTime)
	{
		Update(elapsedTime);
	
		if (m_scriptingContext)
			m_scriptingContext->Update();

		for (auto& layer : m_layers)
		{
			if (layer.IsEnabled())
				layer.PreFrameUpdate(elapsedTime);
		}

		if (m_gamemode)
			m_gamemode->ExecuteCallback<GamemodeEvent::Frame>(elapsedTime);

		for (auto& layer : m_layers)
		{
			if (layer.IsEnabled())
				layer.FrameUpdate(elapsedTime);
		}

		WorldCanvas::OnUpdate(elapsedTime);

		if (m_gamemode)
			m_gamemode->ExecuteCallback<GamemodeEvent::PostFrame>(elapsedTime);

		for (auto& layer : m_layers)
		{
			if (layer.IsEnabled())
				layer.PostFrameUpdate(elapsedTime);
		}
	}

	void MapCanvas::UpdateGrid()
	{
		if (!m_gridEntity)
			return;

		const Camera& camera = GetCamera();

		Nz::Vector2f camPos = camera.GetPosition();
		float zoomFactor = camera.GetZoomFactor();
		Nz::Vector2f size(width() / zoomFactor, height() / zoomFactor);

		auto& gfxComponent = m_gridEntity->GetComponent<Ndk::GraphicsComponent>();
		gfxComponent.Clear();

		auto& nodeComponent = m_gridEntity->GetComponent<Ndk::NodeComponent>();
		nodeComponent.SetPosition(camPos);

		auto AddGrid = [&](float lineWidth, float gridSize, Nz::Color color, int renderOrder)
		{
			Nz::Vector2f gridOffset;
			gridOffset.x = std::fmod(camPos.x, gridSize);
			if (camPos.x < 0.f)
				gridOffset.x += gridSize;

			gridOffset.y = std::fmod(camPos.y, gridSize);
			if (camPos.y < 0.f)
				gridOffset.y += gridSize;

			auto AddLine = [&](const Nz::Vector2f& origin, const Nz::Vector2f& size)
			{
				Nz::SpriteRef lineSprite = Nz::Sprite::New();
				lineSprite->SetColor(color);
				lineSprite->SetSize(size);

				gfxComponent.Attach(lineSprite, Nz::Matrix4f::Translate(origin - gridOffset), renderOrder);
			};

			std::size_t gridCountX = static_cast<std::size_t>(std::ceil(size.x / gridSize)) + 1;
			std::size_t gridCountY = static_cast<std::size_t>(std::ceil(size.y / gridSize)) + 1;

			lineWidth = lineWidth / zoomFactor;

			for (std::size_t x = 0; x < gridCountX; ++x)
				AddLine({ x * gridSize, 0.f }, { lineWidth, size.y + gridOffset.y });

			for (std::size_t y = 0; y < gridCountY; ++y)
				AddLine({ 0.f, y * gridSize }, { size.x + gridOffset.x, lineWidth });
		};

		struct GridZoom
		{
			float gridSize;
			float lineWidth;
			Nz::Color color;
		};

		std::array<GridZoom, 6> gridZooms = {
			{
				{
					512.f,
					1.f,
					Nz::Color(255, 140, 0, 200)
				},
				{
					256.f,
					1.f,
					Nz::Color(0, 139, 139, 200)
				},
				{
					128.f,
					1.f,
					Nz::Color(255, 255, 255, 200)
				},
				{
					64.f,
					1.f,
					Nz::Color(255, 255, 255, 180)
				},
				{
					32.f,
					1.f,
					Nz::Color(255, 255, 255, 127)
				},
				{
					16.f,
					1.f,
					Nz::Color(127, 127, 127, 80)
				}
			}
		};

		for (std::size_t i = 0; i < gridZooms.size(); ++i)
		{
			float screenSize = gridZooms[i].gridSize * zoomFactor;
			if (screenSize >= 32.f)
				AddGrid(gridZooms[i].lineWidth, gridZooms[i].gridSize, gridZooms[i].color, int(std::numeric_limits<int>::lowest() + i * 2));
		}
	}

	std::vector<EntityId> MapCanvas::BuildEntityIds(const std::vector<LayerVisualEntityHandle>& entities)
	{
		std::vector<EntityId> ids;
		ids.reserve(entities.size());

		for (const LayerVisualEntityHandle& visualEntity : entities)
		{
			if (visualEntity)
				ids.push_back(visualEntity->GetUniqueId());
		}

		return ids;
	}
	
	void MapCanvas::resizeEvent(QResizeEvent* event)
	{
		WorldCanvas::resizeEvent(event);

		UpdateGrid();
	}
}
