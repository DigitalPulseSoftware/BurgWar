// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_MAPEDITOR_WIDGETS_LAYERINFODIALOG_HPP
#define BURGWAR_MAPEDITOR_WIDGETS_LAYERINFODIALOG_HPP

#include <CoreLib/LayerIndex.hpp>
#include <Nazara/Core/Color.hpp>
#include <QtGui/QColor>
#include <QtWidgets/QDialog>
#include <tsl/hopscotch_map.h>

class QDoubleSpinBox;
class QLineEdit;
class QPushButton;
class QListWidget;

namespace bw
{
	struct LayerInfo
	{
		Nz::Color backgroundColor;
		std::string name;
	};

	class Map;

	class LayerEditDialog : public QDialog
	{
		public:
			LayerEditDialog(const Map& map, QWidget* parent = nullptr);
			LayerEditDialog(LayerIndex layerIndex, LayerInfo layerInfo, const Map& map, QWidget* parent = nullptr);
			~LayerEditDialog() = default;

			LayerInfo GetLayerInfo() const;

		private:
			void OnAccept();
			void OnEditLayerColor();

			const Map& m_map;
			LayerIndex m_layerIndex;
			QColor m_layerColor;
			QDoubleSpinBox* m_layerDepth;
			QLineEdit* m_layerName;
			QListWidget* m_visibilityList;
			QPushButton* m_layerColorEdit;
	};
}

#include <MapEditor/Widgets/LayerEditDialog.inl>

#endif
