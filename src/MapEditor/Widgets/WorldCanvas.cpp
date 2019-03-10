// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Widgets/WorldCanvas.hpp>
#include <Nazara/Graphics/ColorBackground.hpp>
#include <NDK/Components/CameraComponent.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <NDK/Systems/RenderSystem.hpp>
#include <iostream>

namespace bw
{
	WorldCanvas::WorldCanvas(QWidget* parent) :
	NazaraCanvas(parent)
	{
		Ndk::RenderSystem& renderSystem = m_world.GetSystem<Ndk::RenderSystem>();
		renderSystem.SetGlobalUp(Nz::Vector3f::Down());

		m_cameraEntity = m_world.CreateEntity();
		m_cameraEntity->AddComponent<Ndk::NodeComponent>();

		Ndk::CameraComponent& viewer = m_cameraEntity->AddComponent<Ndk::CameraComponent>();
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

		eventHandler.OnMouseEntered.Connect([this](const Nz::EventHandler*)
		{
			OnMouseEntered();
		});

		eventHandler.OnMouseLeft.Connect([this](const Nz::EventHandler*)
		{
			OnMouseLeft();
		});
		eventHandler.OnMouseMoved.Connect([this](const Nz::EventHandler*, const Nz::WindowEvent::MouseMoveEvent& mouseMoved)
		{
			OnMouseMoved(mouseMoved);
		});
	}

	void WorldCanvas::EnableCameraControl(bool enable)
	{
		if (m_cameraMovement.has_value() == enable)
			return;

		if (enable)
		{
			m_cameraMovement.emplace(GetCursorController(), GetCameraEntity());
			m_cameraMovement->OnCameraMoved.Connect([this](CameraMovement*)
			{
				OnCameraMoved(this);
			});
		}
		else
			m_cameraMovement.reset();
	}

	void WorldCanvas::UpdateBackgroundColor(Nz::Color color)
	{
		m_world.GetSystem<Ndk::RenderSystem>().SetDefaultBackground(Nz::ColorBackground::New(color));
	}

	void WorldCanvas::OnUpdate(float elapsedTime)
	{
		SetActive(true);
		m_world.Update(elapsedTime);

		NazaraCanvas::OnUpdate(elapsedTime);
		SetActive(false);
	}

	void WorldCanvas::OnMouseButtonPressed(const Nz::WindowEvent::MouseButtonEvent& mouseButton)
	{
		if (m_cameraMovement && m_cameraMovement->OnMouseButtonPressed(mouseButton))
			return;
	}

	void WorldCanvas::OnMouseButtonReleased(const Nz::WindowEvent::MouseButtonEvent& mouseButton)
	{
		if (m_cameraMovement && m_cameraMovement->OnMouseButtonReleased(mouseButton))
			return;
	}

	void WorldCanvas::OnMouseEntered()
	{
	}

	void WorldCanvas::OnMouseLeft()
	{
	}

	void WorldCanvas::OnMouseMoved(const Nz::WindowEvent::MouseMoveEvent& mouseMoved)
	{
		if (m_cameraMovement && m_cameraMovement->OnMouseMoved(mouseMoved))
			return;
	}
}