// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Logic/EditorMode.hpp>
#include <iostream>

namespace bw
{
	EditorMode::~EditorMode() = default;

	void EditorMode::OnEnter(EditorWindow& /*editor*/)
	{
	}

	void EditorMode::OnLeave(EditorWindow& /*editor*/)
	{
	}

	void EditorMode::OnMouseButtonPressed(EditorWindow& editor, const Nz::WindowEvent::MouseButtonEvent& /*mouseButton*/)
	{
	}
	
	void EditorMode::OnMouseButtonReleased(EditorWindow& editor, const Nz::WindowEvent::MouseButtonEvent& /*mouseButton*/)
	{
	}
	
	void EditorMode::OnMouseMoved(EditorWindow& editor, const Nz::WindowEvent::MouseMoveEvent& /*mouseMoved*/)
	{
	}
}
