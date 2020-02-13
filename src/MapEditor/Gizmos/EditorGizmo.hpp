// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_MAPEDITOR_EDITORGIZMO_HPP
#define BURGWAR_MAPEDITOR_EDITORGIZMO_HPP

#include <Nazara/Platform/Event.hpp>
#include <NDK/Entity.hpp>

namespace bw
{
	class EditorGizmo
	{
		public:
			inline EditorGizmo(Ndk::Entity* entity);
			virtual ~EditorGizmo();

			inline const Ndk::EntityHandle& GetTargetEntity() const;

			virtual bool OnMouseButtonPressed(const Nz::WindowEvent::MouseButtonEvent& mouseButton) = 0;
			virtual bool OnMouseButtonReleased(const Nz::WindowEvent::MouseButtonEvent& mouseButton) = 0;
			virtual bool OnMouseMoved(const Nz::WindowEvent::MouseMoveEvent& mouseMoved) = 0;

		private:
			Ndk::EntityHandle m_targetEntity;
	};
}

#include <MapEditor/Gizmos/EditorGizmo.inl>

#endif
