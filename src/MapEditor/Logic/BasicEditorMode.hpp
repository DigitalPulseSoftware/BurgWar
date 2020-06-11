// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_MAPEDITOR_SCRIPTING_BASIC_EDITOR_MODE_HPP
#define BURGWAR_MAPEDITOR_SCRIPTING_BASIC_EDITOR_MODE_HPP

#include <MapEditor/Logic/AbstractSelectionEditorMode.hpp>

namespace bw
{
	class BasicEditorMode : public AbstractSelectionEditorMode
	{
		public:
			using AbstractSelectionEditorMode::AbstractSelectionEditorMode;
			~BasicEditorMode() = default;

		private:
			void OnEntityMenu(const QPoint& pos, Ndk::Entity* hoveredEntity) override;
			void OnEntitySelected(Ndk::Entity* selectedEntity) override;
	};
}

#include <MapEditor/Logic/BasicEditorMode.inl>

#endif
