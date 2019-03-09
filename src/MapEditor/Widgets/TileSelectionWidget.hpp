// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_MAPEDITOR_WIDGETS_TILESELECTIONWIDGET_HPP
#define BURGWAR_MAPEDITOR_WIDGETS_TILESELECTIONWIDGET_HPP

#include <QtWidgets/QWidget>

class QScrollBar;

namespace bw
{
	class ScrollCanvas;

	class TileSelectionWidget : public QWidget
	{
		public:
			TileSelectionWidget(QWidget* parent = nullptr);
			~TileSelectionWidget() = default;

		private:
			ScrollCanvas* m_tileSelectionCanvas;
	};
}


#include <MapEditor/Widgets/TileSelectionWidget.inl>

#endif