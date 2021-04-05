// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Logic/BasicEditorMode.hpp>
#include <MapEditor/Widgets/EditorWindow.hpp>
#include <Nazara/Platform/Keyboard.hpp>
#include <NDK/Entity.hpp>

namespace bw
{
	BasicEditorMode::BasicEditorMode(EditorWindow& editor) :
	AbstractSelectionEditorMode(editor),
	m_multiSelectionEnabled(false)
	{
		EditorWindow& editorWindow = GetEditorWindow();
		MapCanvas* mapCanvas = editorWindow.GetMapCanvas();
		m_multiSelectionStateUpdateSlot.Connect(mapCanvas->OnMultiSelectionStateUpdated, [this](MapCanvas* /*emitter*/, bool newState)
		{
			m_multiSelectionEnabled = newState;
		});
	}

	void BasicEditorMode::OnEntityMenu(const QPoint& pos, LayerVisualEntity* hoveredEntity)
	{
		EditorWindow& editorWindow = GetEditorWindow();
		std::optional<std::size_t> entityIndex;
		if (hoveredEntity)
			entityIndex = editorWindow.GetEntityIndex(hoveredEntity->GetUniqueId());

		editorWindow.OpenEntityContextMenu(entityIndex, pos);
	}

	void BasicEditorMode::OnEntitySelected(LayerVisualEntity* selectedEntity)
	{
		EditorWindow& editorWindow = GetEditorWindow();
		if (m_multiSelectionEnabled)
		{
			if (selectedEntity)
				editorWindow.ToggleEntitySelection(editorWindow.GetEntityIndex(selectedEntity->GetUniqueId()));
		}
		else
		{
			if (selectedEntity)
				editorWindow.SelectEntity(editorWindow.GetEntityIndex(selectedEntity->GetUniqueId()));
			else
				editorWindow.ClearSelectedEntity();
		}
	}
}
