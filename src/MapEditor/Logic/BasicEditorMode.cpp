// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Logic/BasicEditorMode.hpp>
#include <MapEditor/Widgets/EditorWindow.hpp>
#include <NDK/Entity.hpp>

namespace bw
{
	void BasicEditorMode::OnEntityMenu(const QPoint& pos, Ndk::Entity* hoveredEntity)
	{
		EditorWindow& editorWindow = GetEditorWindow();
		std::optional<std::size_t> entityIndex;
		if (hoveredEntity)
			entityIndex = editorWindow.GetEntityIndex(hoveredEntity->GetId());

		editorWindow.OpenEntityContextMenu(entityIndex, pos);
	}

	void BasicEditorMode::OnEntitySelected(Ndk::Entity* selectedEntity)
	{
		EditorWindow& editorWindow = GetEditorWindow();
		if (selectedEntity)
			editorWindow.SelectEntity(selectedEntity->GetId());
		else
			editorWindow.ClearSelectedEntity();
	}
}
