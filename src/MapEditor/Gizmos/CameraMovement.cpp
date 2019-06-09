// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Gizmos/CameraMovement.hpp>
#include <Nazara/Math/Ray.hpp>
#include <NDK/Components/CameraComponent.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <NDK/World.hpp>
#include <iostream>

constexpr float zoomLevel = 10.f;
constexpr float maxZoomLevel = zoomLevel - 1.f;
constexpr float minZoomLevel = -zoomLevel;

namespace bw
{
	CameraMovement::CameraMovement(Nz::CursorController& cursorController, Ndk::Entity* camera) :
	m_cursorController(cursorController),
	m_cameraEntity(camera),
	m_isActive(false),
	m_zoomLevel(0.f)
	{
	}

	CameraMovement::~CameraMovement()
	{
	}

	float CameraMovement::ComputeZoomFactor() const
	{
		constexpr float minZoomFactor = 1.f / zoomLevel;
		constexpr float halfZoomLevel = zoomLevel / 2.f;

		return minZoomFactor + (zoomLevel + m_zoomLevel) * minZoomFactor;
	}

	bool CameraMovement::OnMouseButtonPressed(const Nz::WindowEvent::MouseButtonEvent& mouseButton)
	{
		if (mouseButton.button != Nz::Mouse::Right)
			return false;

		m_cursorController.UpdateCursor(Nz::Cursor::Get(Nz::SystemCursor_Move));
		m_isActive = true;

		auto& cameraComponent = m_cameraEntity->GetComponent<Ndk::CameraComponent>();
		m_originalWorldPos = Nz::Vector2f(cameraComponent.Unproject(Nz::Vector3f(float(mouseButton.x), float(mouseButton.y), 0.f)));

		return true;
	}

	bool CameraMovement::OnMouseButtonReleased(const Nz::WindowEvent::MouseButtonEvent& mouseButton)
	{
		if (!m_isActive || mouseButton.button != Nz::Mouse::Right)
			return false;

		m_cursorController.UpdateCursor(Nz::Cursor::Get(Nz::SystemCursor_Default));
		m_isActive = false;

		return true;
	}

	bool CameraMovement::OnMouseMoved(const Nz::WindowEvent::MouseMoveEvent& mouseMoved)
	{
		if (!m_isActive)
			return false;

		auto& cameraComponent = m_cameraEntity->GetComponent<Ndk::CameraComponent>();
		auto& cameraNode = m_cameraEntity->GetComponent<Ndk::NodeComponent>();
		Nz::Vector3f worldPosition = cameraComponent.Unproject(Nz::Vector3f(float(mouseMoved.x), float(mouseMoved.y), 0.f));

		cameraNode.Move(-Nz::Vector2f(worldPosition - m_originalWorldPos), Nz::CoordSys_Global);
		OnCameraMoved(this);

		return true;
	}

	bool CameraMovement::OnMouseWheelMoved(const Nz::WindowEvent::MouseWheelEvent& mouseWheel)
	{
		m_zoomLevel = Nz::Clamp(m_zoomLevel - mouseWheel.delta, minZoomLevel, maxZoomLevel);

		auto& cameraComponent = m_cameraEntity->GetComponent<Ndk::CameraComponent>();
		Nz::Vector2f viewportSize = Nz::Vector2f(cameraComponent.GetTarget()->GetSize());

		cameraComponent.SetSize(ComputeZoomFactor() * viewportSize);
		OnCameraZoomUpdated(this);

		return true;
	}
}