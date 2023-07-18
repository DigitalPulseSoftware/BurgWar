// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_MAPEDITOR_SCRIPTING_EDITOR_MODE_HPP
#define BURGWAR_MAPEDITOR_SCRIPTING_EDITOR_MODE_HPP

#include <Nazara/Platform/WindowEvent.hpp>
#include <memory>

namespace bw
{
	class EditorWindow;

	class EditorMode : public std::enable_shared_from_this<EditorMode>
	{
		public:
			inline EditorMode(EditorWindow& editor);
			virtual ~EditorMode();

			virtual void OnEnter();
			virtual void OnLeave();

			// Canvas events
			virtual void OnMouseButtonPressed(const Nz::WindowEvent::MouseButtonEvent& mouseButton);
			virtual void OnMouseButtonReleased(const Nz::WindowEvent::MouseButtonEvent& mouseButton);
			virtual void OnMouseEntered();
			virtual void OnMouseLeft();
			virtual void OnMouseMoved(const Nz::WindowEvent::MouseMoveEvent& mouseMoved);

		protected:
			inline EditorWindow& GetEditorWindow();
			inline const EditorWindow& GetEditorWindow() const;

		private:
			EditorWindow& m_editor;
	};
}

#include <MapEditor/Logic/EditorMode.inl>

#endif
