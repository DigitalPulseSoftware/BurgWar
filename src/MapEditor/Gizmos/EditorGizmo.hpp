// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_MAPEDITOR_EDITORGIZMO_HPP
#define BURGWAR_MAPEDITOR_EDITORGIZMO_HPP

#include <ClientLib/LayerVisualEntity.hpp>
#include <Nazara/Platform/Event.hpp>
#include <Nazara/Graphics/Model.hpp>
#include <NDK/EntityOwner.hpp>

namespace bw
{
	class EditorGizmo
	{
		public:
			EditorGizmo(entt::registry& renderWorld, std::vector<LayerVisualEntityHandle> entities);
			virtual ~EditorGizmo();

			inline entt::entity GetSelectionOverlayEntity() const;
			inline const std::vector<LayerVisualEntityHandle>& GetTargetEntities() const;

			virtual bool OnMouseButtonPressed(const Nz::WindowEvent::MouseButtonEvent& mouseButton) = 0;
			virtual bool OnMouseButtonReleased(const Nz::WindowEvent::MouseButtonEvent& mouseButton) = 0;
			virtual bool OnMouseMoved(const Nz::WindowEvent::MouseMoveEvent& mouseMoved) = 0;

			void Refresh();

		private:
			static Nz::ModelRef GenerateBoxModel();

			Ndk::EntityOwner m_selectionOverlayEntity;
			std::vector<LayerVisualEntityHandle> m_targetEntities;
	};
}

#include <MapEditor/Gizmos/EditorGizmo.inl>

#endif
