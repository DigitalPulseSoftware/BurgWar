// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Logic/SelectionEditorMode.hpp>
#include <MapEditor/Widgets/EditorWindow.hpp>
#include <MapEditor/Widgets/MapCanvas.hpp>
#include <Nazara/Math/Ray.hpp>
#include <NDK/Components/CameraComponent.hpp>
#include <NDK/Components/GraphicsComponent.hpp>

namespace bw
{
	void SelectionEditorMode::OnEntitySelected(Ndk::Entity* bestEntity)
	{
		if (bestEntity)
			OnSelectEntity(this, bestEntity);
		else
			OnSelectionCleared(this);
	}
}
