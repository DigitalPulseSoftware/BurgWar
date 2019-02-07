// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Widgets/MapWidget.hpp>
#include <Nazara/Graphics/ColorBackground.hpp>
#include <Nazara/Graphics/Sprite.hpp>
#include <Nazara/Math/Ray.hpp>
#include <NDK/Components/CameraComponent.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <NDK/Systems/RenderSystem.hpp>
#include <iostream>

namespace bw
{
	MapWidget::MapWidget(QWidget* parent) :
	NazaraCanvas(parent)
	{
		Ndk::RenderSystem& renderSystem = m_world.GetSystem<Ndk::RenderSystem>();
		renderSystem.SetGlobalUp(Nz::Vector3f::Down());

		m_camera = m_world.CreateEntity();
		m_camera->AddComponent<Ndk::NodeComponent>();

		Ndk::CameraComponent& viewer = m_camera->AddComponent<Ndk::CameraComponent>();
		viewer.SetProjectionType(Nz::ProjectionType_Orthogonal);
		viewer.SetTarget(this);

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

	void MapWidget::ClearEntities()
	{
		for (const Ndk::EntityHandle& entity : m_mapEntities)
			entity->Kill();

		m_mapEntities.Clear();
	}

	Ndk::EntityId MapWidget::CreateEntity(const Nz::Vector2f& position, const Nz::DegreeAnglef& rotation)
	{
		const Ndk::EntityHandle& entity = m_world.CreateEntity();
		m_mapEntities.Insert(entity);

		Nz::SpriteRef pepsi = Nz::Sprite::New();
		pepsi->SetTexture("../resources/burger.png");
		pepsi->SetSize(64.f, 64.f);
		pepsi->SetOrigin(Nz::Vector2f(32.f, 32.f));

		entity->AddComponent<Ndk::GraphicsComponent>().Attach(pepsi);
		auto& nodeComponent = entity->AddComponent<Ndk::NodeComponent>();
		nodeComponent.SetPosition(position);
		nodeComponent.SetRotation(rotation);

		return entity->GetId();
	}

	void MapWidget::EditEntityPosition(Ndk::EntityId entityId)
	{
		const Ndk::EntityHandle& entity = m_world.GetEntity(entityId);
		assert(entity);

		m_positionGizmo = std::make_unique<PositionGizmo>(m_camera, entity, GetEventHandler());
	}

	void MapWidget::UpdateEntityPositionAndRotation(Ndk::EntityId entityId, const Nz::Vector2f& position, const Nz::DegreeAnglef& rotation)
	{
		const Ndk::EntityHandle& entity = m_world.GetEntity(entityId);
		assert(entity);

		auto& nodeComponent = entity->GetComponent<Ndk::NodeComponent>();
		nodeComponent.SetPosition(position);
		nodeComponent.SetRotation(rotation);
	}

	void MapWidget::OnMouseButtonPressed(const Nz::WindowEvent::MouseButtonEvent& mouseButton)
	{
	}

	void MapWidget::OnMouseButtonReleased(const Nz::WindowEvent::MouseButtonEvent& mouseButton)
	{
	}

	void MapWidget::OnMouseMoved(const Nz::WindowEvent::MouseMoveEvent& mouseMoved)
	{
	}

	void MapWidget::OnUpdate(float elapsedTime)
	{
		m_world.Update(elapsedTime);

		NazaraCanvas::OnUpdate(elapsedTime);
	}
}