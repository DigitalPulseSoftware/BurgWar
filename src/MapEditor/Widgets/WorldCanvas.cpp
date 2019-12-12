// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Widgets/WorldCanvas.hpp>
#include <Nazara/Graphics/ColorBackground.hpp>
#include <Nazara/Renderer/DebugDrawer.hpp>
#include <NDK/Components/CameraComponent.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <NDK/Systems/PhysicsSystem2D.hpp>
#include <NDK/Systems/RenderSystem.hpp>

namespace bw
{
	WorldCanvas::WorldCanvas(QWidget* parent) :
	NazaraCanvas(parent),
	m_isPhysicsDebugDrawEnabled(false)
	{
		Ndk::RenderSystem& renderSystem = m_world.GetSystem<Ndk::RenderSystem>();

		m_cameraEntity = m_world.CreateEntity();
		m_cameraEntity->AddComponent<Ndk::NodeComponent>();

		Ndk::CameraComponent& viewer = m_cameraEntity->AddComponent<Ndk::CameraComponent>();
		viewer.SetProjectionType(Nz::ProjectionType_OrthogonalBL);
		viewer.SetTarget(this);

		Nz::EventHandler& eventHandler = GetEventHandler();

		eventHandler.OnKeyPressed.Connect([this](const Nz::EventHandler*, const Nz::WindowEvent::KeyEvent& keyEvent)
		{
			OnKeyPressed(keyEvent);
		});

		eventHandler.OnKeyReleased.Connect([this](const Nz::EventHandler*, const Nz::WindowEvent::KeyEvent& keyEvent)
		{
			OnKeyReleased(keyEvent);
		});

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

		eventHandler.OnMouseWheelMoved.Connect([this](const Nz::EventHandler*, const Nz::WindowEvent::MouseWheelEvent& mouseWheel)
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
			m_cameraMovement.emplace(GetCursorController(), GetCameraEntity());
			m_cameraMovement->OnCameraMoved.Connect([this](CameraMovement*)
			{
				OnCameraMoved(this);
			});
		}
		else
			m_cameraMovement.reset();
	}

	void WorldCanvas::EnablePhysicsDebugDraw(bool enable)
	{
		m_isPhysicsDebugDrawEnabled = enable;
	}

	void WorldCanvas::UpdateBackgroundColor(Nz::Color color)
	{
		m_world.GetSystem<Ndk::RenderSystem>().SetDefaultBackground(Nz::ColorBackground::New(color));
	}

	void WorldCanvas::OnUpdate(float elapsedTime)
	{
		SetActive(true);
		m_world.Update(elapsedTime);

		if (m_isPhysicsDebugDrawEnabled)
		{
			Ndk::PhysicsSystem2D::DebugDrawOptions options;
			options.polygonCallback = [](const Nz::Vector2f* vertices, std::size_t vertexCount, float radius, Nz::Color outline, Nz::Color fillColor, void* userData)
			{
				for (std::size_t i = 0; i < vertexCount - 1; ++i)
					Nz::DebugDrawer::DrawLine(vertices[i], vertices[i + 1]);

				Nz::DebugDrawer::DrawLine(vertices[vertexCount - 1], vertices[0]);
			};

			m_world.GetSystem<Ndk::PhysicsSystem2D>().DebugDraw(options);
		}

		NazaraCanvas::OnUpdate(elapsedTime);
		SetActive(false);
	}

	void WorldCanvas::OnKeyPressed(const Nz::WindowEvent::KeyEvent& key)
	{
	}

	void WorldCanvas::OnKeyReleased(const Nz::WindowEvent::KeyEvent& key)
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