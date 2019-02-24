// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Gizmos/CameraMovement.hpp>
#include <Nazara/Math/Ray.hpp>
#include <NDK/Components/CameraComponent.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <NDK/World.hpp>
#include <iostream>

namespace bw
{
	CameraMovement::CameraMovement(Nz::CursorController& cursorController, Ndk::Entity* camera) :
	m_cursorController(cursorController),
	m_cameraEntity(camera),
	m_isActive(false)
	{
	}

	CameraMovement::~CameraMovement()
	{
	}

	bool CameraMovement::OnMouseButtonPressed(const Nz::WindowEvent::MouseButtonEvent& mouseButton)
	{
		if (mouseButton.button != Nz::Mouse::Right)
			return false;

		m_cursorController.UpdateCursor(Nz::Cursor::Get(Nz::SystemCursor_Move));
		m_isActive = true;

		auto& cameraComponent = m_cameraEntity->GetComponent<Ndk::CameraComponent>();
		m_originalWorldPos = Nz::Vector2f(cameraComponent.Unproject(Nz::Vector3f(mouseButton.x, mouseButton.y, 0.f)));

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
		Nz::Vector3f worldPosition = cameraComponent.Unproject(Nz::Vector3f(mouseMoved.x, mouseMoved.y, 0.f));

		cameraNode.Move(-Nz::Vector2f(worldPosition - m_originalWorldPos), Nz::CoordSys_Global);

		return true;
	}
}