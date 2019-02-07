// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_MAPEDITOR_WIDGETS_POSITIONEDITWIDGET_HPP
#define BURGWAR_MAPEDITOR_WIDGETS_POSITIONEDITWIDGET_HPP

#include <Nazara/Math/Vector2.hpp>
#include <QtWidgets/QWidget>

class QDoubleSpinBox;

namespace bw
{
	class PositionEditWidget : public QWidget
	{
		Q_OBJECT

		public:
			PositionEditWidget(QWidget* parent = nullptr);
			inline PositionEditWidget(const Nz::Vector2d& value, QWidget* parent = nullptr);
			~PositionEditWidget() = default;

			void setValue(const Nz::Vector2d& value);
			Nz::Vector2d value();

		signals:
			void valueChanged(Nz::Vector2d newValue);

		private:
			void onSpinBoxValueChanged();

			QDoubleSpinBox* m_xSpinbox;
			QDoubleSpinBox* m_ySpinbox;
	};
}

#include <MapEditor/Widgets/PositionEditWidget.inl>

#endif