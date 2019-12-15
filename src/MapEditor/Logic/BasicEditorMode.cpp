// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Logic/BasicEditorMode.hpp>
#include <MapEditor/Widgets/EditorWindow.hpp>
#include <NDK/Entity.hpp>

namespace bw
{
	void BasicEditorMode::OnEntitySelected(Ndk::Entity* selectedEntity)
	{
		EditorWindow& editorWindow = GetEditorWindow();
		if (selectedEntity)
			editorWindow.SelectEntity(selectedEntity->GetId());
		else
			editorWindow.ClearSelectedEntity();
	}
}
