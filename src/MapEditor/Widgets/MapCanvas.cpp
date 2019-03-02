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
#include <NDK/Systems/PhysicsSystem2D.hpp>
#include <NDK/Systems/RenderSystem.hpp>
#include <iostream>

namespace bw
{
	MapCanvas::MapCanvas(EditorWindow& editor, QWidget* parent) :
	NazaraCanvas(parent),
	m_editor(editor),
	m_world(false)
	{
		m_world.AddSystem<Ndk::PhysicsSystem2D>();
		Ndk::RenderSystem& renderSystem = m_world.AddSystem<Ndk::RenderSystem>();
		renderSystem.SetGlobalUp(Nz::Vector3f::Down());

		m_cameraEntity = m_world.CreateEntity();
		m_cameraEntity->AddComponent<Ndk::NodeComponent>();

		Ndk::CameraComponent& viewer = m_cameraEntity->AddComponent<Ndk::CameraComponent>();
		viewer.SetProjectionType(Nz::ProjectionType_Orthogonal);
		viewer.SetTarget(this);

		m_cameraMovement.emplace(GetCursorController(), m_cameraEntity);

		Nz::EventHandler& eventHandler = GetEventHandler();

		eventHandler.OnMouseButtonPressed.Connect([this](const Nz::EventHandler*, const Nz::WindowEvent::MouseButtonEvent& mouseButton)
		{
			OnMouseButtonPressed(mouseButton);
		});

		eventHandler.OnMouseButtonReleased.Connect([this](const Nz::EventHandler*, const Nz::WindowEvent::MouseButtonEvent& mouseButton)
		{
			OnMouseButtonReleased(mouseButton);
		});

		eventHandler.OnMouseMoved.Connect([this](const Nz::EventHandler*, const Nz::WindowEvent::MouseMoveEvent& mouseMoved)
		{
			OnMouseMoved(mouseMoved);
		});
	}

	void MapCanvas::ClearEntities()
	{
		for (const Ndk::EntityHandle& entity : m_mapEntities)
			entity->Kill();

		m_mapEntities.Clear();
		m_entityGizmo.reset();
	}

	void MapCanvas::ClearEntitySelection()
	{
		m_entityGizmo.reset();
	}

	Ndk::EntityId MapCanvas::CreateEntity(const std::string& entityClass, const Nz::Vector2f& position, const Nz::DegreeAnglef& rotation, const EntityProperties& properties)
	{
		const ClientEntityStore& entityStore = m_editor.GetEntityStore();

		std::size_t classIndex = entityStore.GetElementIndex(entityClass);
		assert(classIndex != entityStore.InvalidIndex); //< FIXME: This shouldn't crash

		const Ndk::EntityHandle& entity = entityStore.InstantiateEntity(m_world, classIndex, position, rotation, properties);
		m_mapEntities.Insert(entity);

		return entity->GetId();
	}

	void MapCanvas::DeleteEntity(Ndk::EntityId entityId)
	{
		const Ndk::EntityHandle& entity = m_world.GetEntity(entityId);
		assert(entity);

		if (m_entityGizmo && m_entityGizmo->GetTargetEntity() == entity) 
			m_entityGizmo.reset();

		entity->Kill();
	}

	void MapCanvas::EditEntityPosition(Ndk::EntityId entityId)
	{
		const Ndk::EntityHandle& entity = m_world.GetEntity(entityId);
		assert(entity);

		std::unique_ptr<PositionGizmo> positionGizmo = std::make_unique<PositionGizmo>(m_cameraEntity, entity);
		positionGizmo->OnPositionUpdated.Connect([this, entityId](PositionGizmo* /*emitter*/, Nz::Vector2f newPosition)
		{
			OnEntityPositionUpdated(this, entityId, newPosition);
		});

		m_entityGizmo = std::move(positionGizmo);
	}

	void MapCanvas::UpdateBackgroundColor(Nz::Color color)
	{
		m_world.GetSystem<Ndk::RenderSystem>().SetDefaultBackground(Nz::ColorBackground::New(color));
	}

	void MapCanvas::UpdateEntityPositionAndRotation(Ndk::EntityId entityId, const Nz::Vector2f& position, const Nz::DegreeAnglef& rotation)
	{
		const Ndk::EntityHandle& entity = m_world.GetEntity(entityId);
		assert(entity);

		auto& nodeComponent = entity->GetComponent<Ndk::NodeComponent>();
		nodeComponent.SetPosition(position);
		nodeComponent.SetRotation(rotation);
	}

	void MapCanvas::OnMouseButtonPressed(const Nz::WindowEvent::MouseButtonEvent& mouseButton)
	{
		if (m_entityGizmo)
		{
			if (m_entityGizmo->OnMouseButtonPressed(mouseButton))
				return;
		}

		if (m_cameraMovement->OnMouseButtonPressed(mouseButton))
			return;
	}

	void MapCanvas::OnMouseButtonReleased(const Nz::WindowEvent::MouseButtonEvent& mouseButton)
	{
		if (m_entityGizmo)
		{
			if (m_entityGizmo->OnMouseButtonReleased(mouseButton))
				return;
		}

		if (m_cameraMovement->OnMouseButtonReleased(mouseButton))
			return;

		if (mouseButton.button == Nz::Mouse::Left)
		{
			auto& cameraComponent = m_cameraEntity->GetComponent<Ndk::CameraComponent>();
			Nz::Vector3f start = cameraComponent.Unproject(Nz::Vector3f(mouseButton.x, mouseButton.y, 0.f));
			Nz::Vector3f end = cameraComponent.Unproject(Nz::Vector3f(mouseButton.x, mouseButton.y, 1.f));

			Nz::Rayf ray(start, end - start);
			for (const Ndk::EntityHandle& entity : m_mapEntities)
			{
				assert(entity->HasComponent<Ndk::GraphicsComponent>());

				auto& gfxComponent = entity->GetComponent<Ndk::GraphicsComponent>();
				if (ray.Intersect(gfxComponent.GetAABB()))
				{
					OnEntitySelected(this, entity->GetId());
					return;
				}
			}
		}
	}

	void MapCanvas::OnMouseMoved(const Nz::WindowEvent::MouseMoveEvent& mouseMoved)
	{
		if (m_entityGizmo)
		{
			if (m_entityGizmo->OnMouseMoved(mouseMoved))
				return;
		}

		if (m_cameraMovement->OnMouseMoved(mouseMoved))
			return;
	}

	void MapCanvas::OnUpdate(float elapsedTime)
	{
		m_world.Update(elapsedTime);

		NazaraCanvas::OnUpdate(elapsedTime);
	}
}