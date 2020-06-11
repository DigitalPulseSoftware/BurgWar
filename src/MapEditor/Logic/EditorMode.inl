// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Logic/EditorMode.hpp>
#include <cassert>

namespace bw
{
	inline EditorMode::EditorMode(EditorWindow& editor) :
	m_editor(editor)
	{
	}

	inline EditorWindow& EditorMode::GetEditorWindow()
	{
		return m_editor;
	}

	inline const EditorWindow& EditorMode::GetEditorWindow() const
	{
		return m_editor;
	}
}
