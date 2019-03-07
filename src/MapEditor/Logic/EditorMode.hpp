// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_MAPEDITOR_SCRIPTING_EDITOR_MODE_HPP
#define BURGWAR_MAPEDITOR_SCRIPTING_EDITOR_MODE_HPP

#include <Nazara/Platform/Event.hpp>

namespace bw
{
	class EditorWindow;

	class EditorMode
	{
		public:
			EditorMode() = default;
			virtual ~EditorMode();

			virtual void OnEnter(EditorWindow& editor);
			virtual void OnLeave(EditorWindow& editor);

			// Canvas events
			virtual void OnMouseButtonPressed(EditorWindow& editor, const Nz::WindowEvent::MouseButtonEvent& mouseButton);
			virtual void OnMouseButtonReleased(EditorWindow& editor, const Nz::WindowEvent::MouseButtonEvent& mouseButton);
			virtual void OnMouseMoved(EditorWindow& editor, const Nz::WindowEvent::MouseMoveEvent& mouseMoved);
	};
}

#include <MapEditor/Logic/EditorMode.inl>

#endif
