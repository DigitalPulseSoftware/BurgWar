// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Widgets/MapCanvas.hpp>
#include <MapEditor/Gizmos/PositionGizmo.hpp>
#include <MapEditor/Widgets/EditorWindow.hpp>
#include <Nazara/Graphics/ColorBackground.hpp>
#include <Nazara/Graphics/Sprite.hpp>
#include <Nazara/Math/Ray.hpp>
#include <NDK/Components/CameraComponent.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <NDK/Systems/DebugSystem.hpp>
#include <NDK/Systems/PhysicsSystem2D.hpp>
#include <NDK/Systems/RenderSystem.hpp>

namespace bw
{
	MapCanvas::MapCanvas(EditorWindow& editor, QWidget* parent) :
	WorldCanvas(parent),
	m_editor(editor)
	{
		GetWorld().GetSystem<Ndk::DebugSystem>().EnableDepthBuffer(false);

		EnableCameraControl(true);

		GetCameraController().OnCameraZoomUpdated.Connect([this](CameraMovement* controller)
		{
			OnCameraZoomFactorUpdated(this, controller->ComputeZoomFactor());
		});
	}

	void MapCanvas::ClearEntities()
	{
		for (const Ndk::EntityHandle& entity : m_mapEntities)
			entity->Kill();

		m_mapEntities.Clear();

		m_onGizmoEntityDestroyed.Disconnect(); //< Force disconnection because entity destruction does not occur until the world next update
		m_entityGizmo.reset();
	}

	void MapCanvas::ClearEntitySelection()
	{
		m_onGizmoEntityDestroyed.Disconnect();
		m_entityGizmo.reset();
	}

	const Ndk::EntityHandle& MapCanvas::CreateEntity(const std::string& entityClass, const Nz::Vector2f& position, const Nz::DegreeAnglef& rotation, const EntityProperties& properties)
	{
		const EditorEntityStore& entityStore = m_editor.GetEntityStore();

		std::size_t classIndex = entityStore.GetElementIndex(entityClass);
		assert(classIndex != entityStore.InvalidIndex); //< FIXME: This shouldn't crash

		const Ndk::EntityHandle& entity = entityStore.InstantiateEntity(GetWorld(), classIndex, position, rotation, properties);
		if (entity)
		{
			m_mapEntities.Insert(entity);

			return entity;
		}
		else
		{
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

		std::unique_ptr<PositionGizmo> positionGizmo = std::make_unique<PositionGizmo>(GetCameraEntity(), entity);
		positionGizmo->OnPositionUpdated.Connect([this, entityId](PositionGizmo* /*emitter*/, Nz::Vector2f newPosition)
		{
			OnEntityPositionUpdated(this, entityId, newPosition);
		});

		m_entityGizmo = std::move(positionGizmo);
		m_onGizmoEntityDestroyed.Connect(entity->OnEntityDestruction, [this](Ndk::Entity* entity)
		{
			assert(m_entityGizmo->GetTargetEntity() == entity);
			NazaraUnused(entity);

			m_entityGizmo.reset();
		});
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
		switch (key.code)
		{
			case Nz::Keyboard::Key::Delete:
			{
				if (m_entityGizmo)
					OnDeleteEntity(this, m_entityGizmo->GetTargetEntity()->GetId());

				break;
			}

			default:
				break;
		}
	}

	void MapCanvas::OnKeyReleased(const Nz::WindowEvent::KeyEvent& key)
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
}