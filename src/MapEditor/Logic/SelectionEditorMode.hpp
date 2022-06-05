// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_MAPEDITOR_SCRIPTING_SELECTION_EDITOR_MODE_HPP
#define BURGWAR_MAPEDITOR_SCRIPTING_SELECTION_EDITOR_MODE_HPP

#include <MapEditor/Logic/AbstractSelectionEditorMode.hpp>
#include <Nazara/Utils/Signal.hpp>

namespace bw
{
	class SelectionEditorMode final : public AbstractSelectionEditorMode
	{
		public:
			using AbstractSelectionEditorMode::AbstractSelectionEditorMode;
			~SelectionEditorMode() = default;

			NazaraSignal(OnOpenMenu, SelectionEditorMode* /*editorMode*/, const QPoint& /*pos*/, LayerVisualEntity* /*selectedEntity*/);
			NazaraSignal(OnSelectEntity, SelectionEditorMode* /*editorMode*/, LayerVisualEntity* /*selectedEntity*/);
			NazaraSignal(OnSelectionCleared, SelectionEditorMode* /*editorMode*/);

		private:
			void OnEntityMenu(const QPoint& pos, LayerVisualEntity* hoveredEntity) override;
			void OnEntitySelected(LayerVisualEntity* selectedEntity) override;
	};
}

#include <MapEditor/Logic/SelectionEditorMode.inl>

#endif
