// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Logic/EditorMode.hpp>

namespace bw
{
	EditorMode::~EditorMode() = default;

	void EditorMode::OnEnter()
	{
	}

	void EditorMode::OnLeave()
	{
	}

	void EditorMode::OnMouseButtonPressed(const Nz::WindowEvent::MouseButtonEvent& /*mouseButton*/)
	{
	}
	
	void EditorMode::OnMouseButtonReleased(const Nz::WindowEvent::MouseButtonEvent& /*mouseButton*/)
	{
	}

	void EditorMode::OnMouseEntered()
	{
	}

	void EditorMode::OnMouseLeft()
	{
	}
	
	void EditorMode::OnMouseMoved(const Nz::WindowEvent::MouseMoveEvent& /*mouseMoved*/)
	{
	}
}
