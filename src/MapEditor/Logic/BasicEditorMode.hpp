// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_MAPEDITOR_SCRIPTING_BASIC_EDITOR_MODE_HPP
#define BURGWAR_MAPEDITOR_SCRIPTING_BASIC_EDITOR_MODE_HPP

#include <MapEditor/Logic/AbstractSelectionEditorMode.hpp>
#include <MapEditor/Widgets/MapCanvas.hpp>

namespace bw
{
	class BasicEditorMode : public AbstractSelectionEditorMode
	{
		public:
			BasicEditorMode(EditorWindow& editor);
			~BasicEditorMode() = default;

		private:
			void OnEntityMenu(const QPoint& pos, LayerVisualEntity* hoveredEntity) override;
			void OnEntitySelected(LayerVisualEntity* selectedEntity) override;

			NazaraSlot(MapCanvas, OnMultiSelectionStateUpdated, m_multiSelectionStateUpdateSlot);

			bool m_multiSelectionEnabled;
	};
}

#include <MapEditor/Logic/BasicEditorMode.inl>

#endif
