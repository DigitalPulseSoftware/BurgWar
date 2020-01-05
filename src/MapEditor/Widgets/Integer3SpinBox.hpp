// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_MAPEDITOR_WIDGETS_INTEGER3SPINBOX_HPP
#define BURGWAR_MAPEDITOR_WIDGETS_INTEGER3SPINBOX_HPP

#include <Nazara/Math/Vector3.hpp>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QWidget>

class QSpinBox;

namespace bw
{
	class Integer3SpinBox : public QWidget
	{
		Q_OBJECT

		public:
			enum class LabelMode
			{
				NoLabel,
				EulerAngles,
				PositionLabel,
				ScaleLabel,
				SizeLabel
			};

			Integer3SpinBox(LabelMode labelMode, QBoxLayout::Direction dir, QWidget* parent = nullptr);
			inline Integer3SpinBox(const Nz::Vector3i64& value, LabelMode labelMode, QBoxLayout::Direction dir, QWidget* parent = nullptr);
			~Integer3SpinBox() = default;

			void setValue(const Nz::Vector3i64& value);
			Nz::Vector3i64 value();

		signals:
			void valueChanged(Nz::Vector3i64 newValue);

		private:
			void onSpinBoxValueChanged();

			QSpinBox* m_xSpinbox;
			QSpinBox* m_ySpinbox;
			QSpinBox* m_zSpinbox;
			bool m_ignoreSignal;
	};
}

#include <MapEditor/Widgets/Integer3SpinBox.inl>

#endif