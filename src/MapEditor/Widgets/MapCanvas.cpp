// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Widgets/MapCanvas.hpp>
#include <CoreLib/LogSystem/Logger.hpp>
#include <MapEditor/Gizmos/PositionGizmo.hpp>
#include <MapEditor/Widgets/EditorWindow.hpp>
#include <Nazara/Graphics/ColorBackground.hpp>
#include <Nazara/Graphics/Sprite.hpp>
#include <Nazara/Math/Ray.hpp>
#include <NDK/Components/CameraComponent.hpp>
#include <NDK/Components/GraphicsComponent.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <NDK/Systems/PhysicsSystem2D.hpp>
#include <NDK/Systems/RenderSystem.hpp>

namespace bw
{
	MapCanvas::MapCanvas(EditorWindow& editor, QWidget* parent) :
	WorldCanvas(parent),
	m_editor(editor)
	{
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
	}

	void MapCanvas::ClearEntities()
	{
		for (const Ndk::EntityHandle& entity : m_mapEntities)
			entity->Kill();

		m_mapEntities.Clear();

		ClearEntitySelection(); //< Force disconnection because entity destruction does not occur until the world next update
	}

	void MapCanvas::ClearEntitySelection()
	{
		m_onGizmoEntityDestroyed.Disconnect();
		m_onLayerAlignmentUpdate.Disconnect(); //< Used by position gizmo
		m_entityGizmo.reset();
	}

	const Ndk::EntityHandle& MapCanvas::CreateEntity(const std::string& entityClass, const Nz::Vector2f& position, const Nz::DegreeAnglef& rotation, const PropertyValueMap& properties)
	{
		const EditorEntityStore& entityStore = m_editor.GetEntityStore();

		std::size_t classIndex = entityStore.GetElementIndex(entityClass);

		try
		{
			if (classIndex == entityStore.InvalidIndex)
				throw std::runtime_error("entity class is not registered");

			const Ndk::EntityHandle& entity = entityStore.InstantiateEntity(GetWorld(), classIndex, position, rotation, 1.f, properties);
			if (!entity)
				throw std::runtime_error("failed to instantiate entity");

			assert(entity);
				
			m_mapEntities.Insert(entity);

			return entity;
		}
		catch (const std::exception& e)
		{
			bwLog(m_editor.GetLogger(), LogLevel::Error, "Failed to instantiate entity of type {}: {}", entityClass, e.what());

			const Ndk::EntityHandle& dummyEntity = GetWorld().CreateEntity();
			dummyEntity->AddComponent<Ndk::GraphicsComponent>();
			dummyEntity->AddComponent<Ndk::NodeComponent>();

			m_mapEntities.Insert(dummyEntity);

			return dummyEntity;
		}
	}

	void MapCanvas::DeleteEntity(Ndk::EntityId entityId)
	{
		const Ndk::EntityHandle& entity = GetWorld().GetEntity(entityId);
		assert(entity);

		entity->Kill();
	}

	void MapCanvas::EditEntityPosition(Ndk::EntityId entityId)
	{
		const Ndk::EntityHandle& entity = GetWorld().GetEntity(entityId);
		if (!entity)
			return;

		const Map& mapData = m_editor.GetWorkingMap();
		const auto& currentLayerOpt = m_editor.GetCurrentLayer();
		assert(currentLayerOpt);

		const Map::Layer& layerData = mapData.GetLayer(*currentLayerOpt);

		std::unique_ptr<PositionGizmo> positionGizmo = std::make_unique<PositionGizmo>(GetCamera(), entity, layerData.positionAlignment);
		positionGizmo->OnPositionUpdated.Connect([this, entityId](PositionGizmo* /*emitter*/, Nz::Vector2f newPosition)
		{
			OnEntityPositionUpdated(this, entityId, newPosition);
		});

		m_onLayerAlignmentUpdate.Connect(m_editor.OnLayerAlignmentUpdate, [this, gizmo = positionGizmo.get()](EditorWindow* /*editor*/, std::size_t layerIndex, const Nz::Vector2f& newAlignment)
		{
			auto currentLayerOpt = m_editor.GetCurrentLayer();
			if (!currentLayerOpt || *currentLayerOpt != layerIndex)
				return;

			gizmo->UpdatePositionAlignment(newAlignment);
		});

		m_entityGizmo = std::move(positionGizmo);
		m_onGizmoEntityDestroyed.Connect(entity->OnEntityDestruction, [this](Ndk::Entity* entity)
		{
			assert(m_entityGizmo->GetTargetEntity() == entity);
			NazaraUnused(entity);

			ClearEntitySelection();
		});
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

	void MapCanvas::UpdateEntityPositionAndRotation(Ndk::EntityId entityId, const Nz::Vector2f& position, const Nz::DegreeAnglef& rotation)
	{
		const Ndk::EntityHandle& entity = GetWorld().GetEntity(entityId);
		assert(entity);

		auto& nodeComponent = entity->GetComponent<Ndk::NodeComponent>();
		nodeComponent.SetPosition(position);
		nodeComponent.SetRotation(rotation);
	}

	void MapCanvas::OnKeyPressed(const Nz::WindowEvent::KeyEvent& key)
	{
		switch (key.virtualKey)
		{
			case Nz::Keyboard::VKey::Delete:
			{
				if (m_entityGizmo)
					OnDeleteEntity(this, m_entityGizmo->GetTargetEntity()->GetId());

				break;
			}

			default:
				break;
		}
	}

	void MapCanvas::OnKeyReleased(const Nz::WindowEvent::KeyEvent& /*key*/)
	{
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
	
	void MapCanvas::resizeEvent(QResizeEvent* event)
	{
		WorldCanvas::resizeEvent(event);

		UpdateGrid();
	}
}
