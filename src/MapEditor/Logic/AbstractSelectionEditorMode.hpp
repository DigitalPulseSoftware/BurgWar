// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_MAPEDITOR_SCRIPTING_ABSTRACT_SELECTION_EDITOR_MODE_HPP
#define BURGWAR_MAPEDITOR_SCRIPTING_ABSTRACT_SELECTION_EDITOR_MODE_HPP

#include <MapEditor/Logic/EditorMode.hpp>
#include <Nazara/Math/Vector2.hpp>
#include <QtCore/QPoint>
#include <optional>

namespace bw
{
	class LayerVisualEntity;

	class AbstractSelectionEditorMode : public EditorMode
	{
		public:
			using EditorMode::EditorMode;
			~AbstractSelectionEditorMode() = default;

			void OnMouseButtonPressed(const Nz::WindowEvent::MouseButtonEvent& mouseButton) override;
			void OnMouseButtonReleased(const Nz::WindowEvent::MouseButtonEvent& mouseButton) override;

		protected:
			virtual void OnEntityMenu(const QPoint& /*pos*/, LayerVisualEntity* /*hoveredEntity*/) = 0;
			virtual void OnEntitySelected(LayerVisualEntity* /*selectedEntity*/) = 0;

		private:
			std::optional<Nz::Vector2i> m_rightClickBegin;
	};
}

#include <MapEditor/Logic/AbstractSelectionEditorMode.inl>

#endif
