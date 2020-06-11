// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_MAPEDITOR_GIZMOS_CAMERA_HPP
#define BURGWAR_MAPEDITOR_GIZMOS_CAMERA_HPP

#include <Nazara/Math/Vector2.hpp>
#include <Nazara/Platform/CursorController.hpp>
#include <Nazara/Platform/Event.hpp>
#include <NDK/Entity.hpp>

namespace bw
{
	class Camera;

	class CameraMovement
	{
		public:
			CameraMovement(Nz::CursorController& cursorController, Camera& camera);
			~CameraMovement();

			float ComputeZoomFactor() const;

			inline float GetZoomLevel() const;

			bool OnMouseButtonPressed(const Nz::WindowEvent::MouseButtonEvent& mouseButton);
			bool OnMouseButtonReleased(const Nz::WindowEvent::MouseButtonEvent& mouseButton);
			bool OnMouseMoved(const Nz::WindowEvent::MouseMoveEvent& mouseMoved);
			bool OnMouseWheelMoved(const Nz::WindowEvent::MouseWheelEvent& mouseWheel);

			NazaraSignal(OnCameraMoved, CameraMovement* /*emitter*/);
			NazaraSignal(OnCameraZoomUpdated, CameraMovement* /*emitter*/);

		private:
			Nz::CursorController& m_cursorController;
			Nz::Vector2f m_originalWorldPos;
			Camera& m_camera;
			bool m_isActive;
			float m_zoomLevel;
	};
}

#include <MapEditor/Gizmos/CameraMovement.inl>

#endif
