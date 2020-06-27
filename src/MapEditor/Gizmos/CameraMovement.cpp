// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Gizmos/CameraMovement.hpp>
#include <ClientLib/Camera.hpp>
#include <Nazara/Math/Ray.hpp>
#include <NDK/World.hpp>

constexpr float zoomLevel = 10.f;
constexpr float maxZoomLevel = zoomLevel;
constexpr float minZoomLevel = -zoomLevel + 1;

namespace bw
{
	CameraMovement::CameraMovement(Nz::CursorController& cursorController, Camera& camera) :
	m_cursorController(cursorController),
	m_camera(camera),
	m_isActive(false),
	m_zoomLevel(0.f)
	{
	}

	CameraMovement::~CameraMovement() = default;

	float CameraMovement::ComputeZoomFactor() const
	{
		constexpr float minZoomFactor = 1.f / zoomLevel;

		return minZoomFactor + (zoomLevel - m_zoomLevel) * minZoomFactor;
	}

	bool CameraMovement::OnMouseButtonPressed(const Nz::WindowEvent::MouseButtonEvent& mouseButton)
	{
		if (mouseButton.button != Nz::Mouse::Middle && mouseButton.button != Nz::Mouse::Right)
			return false;

		m_cursorController.UpdateCursor(Nz::Cursor::Get(Nz::SystemCursor_Move));
		m_isActive = true;

		m_originalWorldPos = m_camera.Unproject({ float(mouseButton.x), float(mouseButton.y) });
		return true;
	}

	bool CameraMovement::OnMouseButtonReleased(const Nz::WindowEvent::MouseButtonEvent& mouseButton)
	{
		if (mouseButton.button != Nz::Mouse::Middle && mouseButton.button != Nz::Mouse::Right)
			return false;

		m_cursorController.UpdateCursor(Nz::Cursor::Get(Nz::SystemCursor_Default));
		m_isActive = false;

		return true;
	}

	bool CameraMovement::OnMouseMoved(const Nz::WindowEvent::MouseMoveEvent& mouseMoved)
	{
		if (!m_isActive)
			return false;

		Nz::Vector3f worldPosition = m_camera.Unproject({ float(mouseMoved.x), float(mouseMoved.y) });

		m_camera.MoveBy(-Nz::Vector2f(worldPosition - m_originalWorldPos));
		OnCameraMoved(this);

		return true;
	}

	bool CameraMovement::OnMouseWheelMoved(const Nz::WindowEvent::MouseWheelEvent& mouseWheel)
	{
		m_zoomLevel = Nz::Clamp(m_zoomLevel - mouseWheel.delta, minZoomLevel, maxZoomLevel);

		m_camera.SetZoomFactor(ComputeZoomFactor());
		OnCameraZoomUpdated(this);

		return true;
	}
}
