// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_MAPEDITOR_WIDGETS_FLOAT3SPINBOX_HPP
#define BURGWAR_MAPEDITOR_WIDGETS_FLOAT3SPINBOX_HPP

#include <Nazara/Math/Vector3.hpp>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QWidget>

class QDoubleSpinBox;

namespace bw
{
	class Float3SpinBox : public QWidget
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

			Float3SpinBox(LabelMode labelMode, QBoxLayout::Direction dir, QWidget* parent = nullptr);
			inline Float3SpinBox(const Nz::Vector3f& value, LabelMode labelMode, QBoxLayout::Direction dir, QWidget* parent = nullptr);
			~Float3SpinBox() = default;

			void setValue(const Nz::Vector3f& value);
			Nz::Vector3f value();

		signals:
			void valueChanged(Nz::Vector3f newValue);

		private:
			void onSpinBoxValueChanged();

			QDoubleSpinBox* m_xSpinbox;
			QDoubleSpinBox* m_ySpinbox;
			QDoubleSpinBox* m_zSpinbox;
			bool m_ignoreSignal;
	};
}

#include <MapEditor/Widgets/Float3SpinBox.inl>

#endif
