// Copyright (C) 2019 Jérôme Leclercq
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
	class CameraMovement
	{
		friend class MapCanvas;

		public:
			CameraMovement(Nz::CursorController& cursorController, Ndk::Entity* camera);
			~CameraMovement();

		private:
			bool OnMouseButtonPressed(const Nz::WindowEvent::MouseButtonEvent& mouseButton);
			bool OnMouseButtonReleased(const Nz::WindowEvent::MouseButtonEvent& mouseButton);
			bool OnMouseMoved(const Nz::WindowEvent::MouseMoveEvent& mouseMoved);

			Nz::CursorController& m_cursorController;
			Nz::Vector2f m_originalWorldPos;
			Ndk::EntityHandle m_cameraEntity;
			bool m_isActive;
	};
}

#include <MapEditor/Gizmos/CameraMovement.inl>

#endif