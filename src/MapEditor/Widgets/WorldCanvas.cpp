// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Widgets/WorldCanvas.hpp>
#include <Nazara/Graphics/ColorBackground.hpp>
#include <NDK/Components/CameraComponent.hpp>
#include <Nazara/Utility/Components/NodeComponent.hpp>
#include <NDK/Systems/PhysicsSystem2D.hpp>
#include <NDK/Systems/RenderSystem.hpp>

namespace bw
{
	WorldCanvas::WorldCanvas(QWidget* parent) :
	NazaraCanvas(parent),
	m_backgroundColor(Nz::Color::Black()),
	m_camera(m_world, this, true)
	{
		Ndk::RenderSystem& renderSystem = m_world.AddSystem<Ndk::RenderSystem>();
		renderSystem.SetGlobalUp(Nz::Vector3f::Down());

		Nz::WindowEventHandler& eventHandler = GetEventHandler();

		eventHandler.OnKeyPressed.Connect([this](const Nz::WindowEventHandler*, const Nz::WindowEvent::KeyEvent& keyEvent)
		{
			OnKeyPressed(keyEvent);
		});

		eventHandler.OnKeyReleased.Connect([this](const Nz::WindowEventHandler*, const Nz::WindowEvent::KeyEvent& keyEvent)
		{
			OnKeyReleased(keyEvent);
		});

		eventHandler.OnMouseButtonPressed.Connect([this](const Nz::WindowEventHandler*, const Nz::WindowEvent::MouseButtonEvent& mouseButton)
		{
			OnMouseButtonPressed(mouseButton);
		});

		eventHandler.OnMouseButtonReleased.Connect([this](const Nz::WindowEventHandler*, const Nz::WindowEvent::MouseButtonEvent& mouseButton)
		{
			OnMouseButtonReleased(mouseButton);
		});

		eventHandler.OnMouseEntered.Connect([this](const Nz::WindowEventHandler*)
		{
			OnMouseEntered();
		});

		eventHandler.OnMouseLeft.Connect([this](const Nz::WindowEventHandler*)
		{
			OnMouseLeft();
		});

		eventHandler.OnMouseMoved.Connect([this](const Nz::WindowEventHandler*, const Nz::WindowEvent::MouseMoveEvent& mouseMoved)
		{
			OnMouseMoved(mouseMoved);
		});

		eventHandler.OnMouseWheelMoved.Connect([this](const Nz::WindowEventHandler*, const Nz::WindowEvent::MouseWheelEvent& mouseWheel)
		{
			OnMouseWheelMoved(mouseWheel);
		});
	}

	void WorldCanvas::EnableCameraControl(bool enable)
	{
		if (m_cameraMovement.has_value() == enable)
			return;

		if (enable)
		{
			m_cameraMovement.emplace(GetCursorController(), GetCamera());
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
		OnBackgroundColorUpdate(this, color);
		m_backgroundColor = color;
		m_world.GetSystem<Ndk::RenderSystem>().SetDefaultBackground(Nz::ColorBackground::New(color));
	}

	void WorldCanvas::OnUpdate(Nz::Time elapsedTime)
	{
		SetActive(true);
		m_world.Update(elapsedTime);

		NazaraCanvas::OnUpdate(elapsedTime);
		SetActive(false);
	}

	void WorldCanvas::OnKeyPressed(const Nz::WindowEvent::KeyEvent& /*key*/)
	{
	}

	void WorldCanvas::OnKeyReleased(const Nz::WindowEvent::KeyEvent& /*key*/)
	{
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

	void WorldCanvas::OnMouseWheelMoved(const Nz::WindowEvent::MouseWheelEvent& mouseWheel)
	{
		if (m_cameraMovement && m_cameraMovement->OnMouseWheelMoved(mouseWheel))
			return;
	}
}
