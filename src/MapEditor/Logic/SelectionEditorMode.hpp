// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_MAPEDITOR_SCRIPTING_SELECTION_EDITOR_MODE_HPP
#define BURGWAR_MAPEDITOR_SCRIPTING_SELECTION_EDITOR_MODE_HPP

#include <MapEditor/Logic/AbstractSelectionEditorMode.hpp>
#include <Nazara/Core/Signal.hpp>

namespace bw
{
	class SelectionEditorMode final : public AbstractSelectionEditorMode
	{
		public:
			using AbstractSelectionEditorMode::AbstractSelectionEditorMode;
			~SelectionEditorMode() = default;

			NazaraSignal(OnOpenMenu, SelectionEditorMode* /*editorMode*/, const QPoint& /*pos*/, Ndk::Entity* /*selectedEntity*/);
			NazaraSignal(OnSelectEntity, SelectionEditorMode* /*editorMode*/, Ndk::Entity* /*selectedEntity*/);
			NazaraSignal(OnSelectionCleared, SelectionEditorMode* /*editorMode*/);

		private:
			void OnEntityMenu(const QPoint& pos, Ndk::Entity* hoveredEntity) override;
			void OnEntitySelected(Ndk::Entity* selectedEntity) override;
	};
}

#include <MapEditor/Logic/SelectionEditorMode.inl>

#endif
