// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_MAPEDITOR_WIDGETS_TILEMAPEDITORDIALOG_HPP
#define BURGWAR_MAPEDITOR_WIDGETS_TILEMAPEDITORDIALOG_HPP

#include <Nazara/Graphics/TileMap.hpp>
#include <Nazara/Math/Vector2.hpp>
#include <QtWidgets/QDialog>
#include <vector>

class QScrollBar;
class QWidget;

namespace bw
{
	class ScrollCanvas;

	class TileMapEditorDialog : public QDialog
	{
		public:
			TileMapEditorDialog(const Nz::Vector2ui& mapSize, const Nz::Vector2f& tileSize, const std::vector<Nz::UInt32>& content, QWidget* parent = nullptr);
			~TileMapEditorDialog() = default;

		private:
			Nz::TileMapRef m_tileMap;
			ScrollCanvas* m_tileSelectionCanvas;
			ScrollCanvas* m_tilemapCanvas;
	};
}


#include <MapEditor/Widgets/TileMapEditorDialog.inl>

#endif