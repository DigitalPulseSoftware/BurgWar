// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_MAPEDITOR_SCRIPTING_BASIC_EDITOR_MODE_HPP
#define BURGWAR_MAPEDITOR_SCRIPTING_BASIC_EDITOR_MODE_HPP

#include <MapEditor/Logic/EditorMode.hpp>

namespace bw
{
	class BasicEditorMode : public EditorMode
	{
		public:
			BasicEditorMode() = default;
			~BasicEditorMode() = default;

			void OnMouseButtonReleased(EditorWindow& editor, const Nz::WindowEvent::MouseButtonEvent& mouseButton) override;
	};
}

#include <MapEditor/Logic/BasicEditorMode.inl>

#endif
