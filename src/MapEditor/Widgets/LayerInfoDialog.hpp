// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_MAPEDITOR_WIDGETS_LAYERINFODIALOG_HPP
#define BURGWAR_MAPEDITOR_WIDGETS_LAYERINFODIALOG_HPP

#include <Nazara/Core/Color.hpp>
#include <QtGui/QColor>
#include <QtWidgets/QDialog>
#include <tsl/hopscotch_map.h>

class QDoubleSpinBox;
class QLineEdit;
class QPushButton;

namespace bw
{
	struct LayerInfo
	{
		Nz::Color backgroundColor;
		std::string name;
		float depth;
	};

	class LayerInfoDialog : public QDialog
	{
		public:
			LayerInfoDialog(QWidget* parent = nullptr);
			LayerInfoDialog(LayerInfo layerInfo, QWidget* parent = nullptr);
			~LayerInfoDialog() = default;

			LayerInfo GetLayerInfo() const;

		private:
			void OnAccept();
			void OnEditLayerColor();

			QColor m_layerColor;
			QDoubleSpinBox* m_layerDepth;
			QLineEdit* m_layerName;
			QPushButton* m_layerColorEdit;
	};
}

#include <MapEditor/Widgets/LayerInfoDialog.inl>

#endif