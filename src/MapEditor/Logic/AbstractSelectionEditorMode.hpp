// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_MAPEDITOR_SCRIPTING_ABSTRACT_SELECTION_EDITOR_MODE_HPP
#define BURGWAR_MAPEDITOR_SCRIPTING_ABSTRACT_SELECTION_EDITOR_MODE_HPP

#include <MapEditor/Logic/EditorMode.hpp>
#include <Nazara/Math/Vector2.hpp>
#include <QtCore/QPoint>

namespace Ndk
{
	class Entity;
}

namespace bw
{
	class AbstractSelectionEditorMode : public EditorMode
	{
		public:
			using EditorMode::EditorMode;
			~AbstractSelectionEditorMode() = default;

			void OnMouseButtonReleased(const Nz::WindowEvent::MouseButtonEvent& mouseButton) override;

		protected:
			virtual void OnEntityMenu(const QPoint& /*pos*/, Ndk::Entity* /*hoveredEntity*/) = 0;
			virtual void OnEntitySelected(Ndk::Entity* /*selectedEntity*/) = 0;
	};
}

#include <MapEditor/Logic/AbstractSelectionEditorMode.inl>

#endif
