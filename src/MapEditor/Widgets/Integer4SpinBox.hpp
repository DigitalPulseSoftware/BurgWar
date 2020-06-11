// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_MAPEDITOR_WIDGETS_INTEGER4SPINBOX_HPP
#define BURGWAR_MAPEDITOR_WIDGETS_INTEGER4SPINBOX_HPP

#include <Nazara/Math/Vector4.hpp>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QWidget>

class QSpinBox;

namespace bw
{
	class Integer4SpinBox : public QWidget
	{
		Q_OBJECT

		public:
			enum class LabelMode
			{
				NoLabel,
				PositionLabel,
				RectLabel
			};

			Integer4SpinBox(LabelMode labelMode, QBoxLayout::Direction dir, QWidget* parent = nullptr);
			inline Integer4SpinBox(const Nz::Vector4i64& value, LabelMode labelMode, QBoxLayout::Direction dir, QWidget* parent = nullptr);
			~Integer4SpinBox() = default;

			void setValue(const Nz::Vector4i64& value);
			Nz::Vector4i64 value();

		signals:
			void valueChanged(Nz::Vector4i64 newValue);

		private:
			void onSpinBoxValueChanged();

			QSpinBox* m_xSpinbox;
			QSpinBox* m_ySpinbox;
			QSpinBox* m_zSpinbox;
			QSpinBox* m_wSpinbox;
			bool m_ignoreSignal;
	};
}

#include <MapEditor/Widgets/Integer4SpinBox.inl>

#endif
