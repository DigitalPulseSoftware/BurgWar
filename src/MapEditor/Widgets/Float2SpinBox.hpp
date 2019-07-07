// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_MAPEDITOR_WIDGETS_POSITIONEDITWIDGET_HPP
#define BURGWAR_MAPEDITOR_WIDGETS_POSITIONEDITWIDGET_HPP

#include <Nazara/Math/Vector2.hpp>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QWidget>

class QDoubleSpinBox;

namespace bw
{
	class Float2SpinBox : public QWidget
	{
		Q_OBJECT

		public:
			enum class LabelMode
			{
				NoLabel,
				PositionLabel,
				SizeLabel
			};

			Float2SpinBox(LabelMode labelMode, QBoxLayout::Direction dir, QWidget* parent = nullptr);
			inline Float2SpinBox(const Nz::Vector2f& value, LabelMode labelMode, QBoxLayout::Direction dir, QWidget* parent = nullptr);
			~Float2SpinBox() = default;

			void setValue(const Nz::Vector2f& value);
			Nz::Vector2f value();

		signals:
			void valueChanged(Nz::Vector2f newValue);

		private:
			void onSpinBoxValueChanged();

			QDoubleSpinBox* m_xSpinbox;
			QDoubleSpinBox* m_ySpinbox;
			bool m_ignoreSignal;
	};
}

#include <MapEditor/Widgets/Float2SpinBox.inl>

#endif