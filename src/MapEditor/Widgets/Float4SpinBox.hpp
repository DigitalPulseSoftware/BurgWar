// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_MAPEDITOR_WIDGETS_FLOAT4SPINBOX_HPP
#define BURGWAR_MAPEDITOR_WIDGETS_FLOAT4SPINBOX_HPP

#include <Nazara/Math/Vector4.hpp>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QWidget>

class QDoubleSpinBox;

namespace bw
{
	class Float4SpinBox : public QWidget
	{
		Q_OBJECT

		public:
			enum class LabelMode
			{
				NoLabel,
				PositionLabel,
				RectLabel
			};

			Float4SpinBox(LabelMode labelMode, QBoxLayout::Direction dir, QWidget* parent = nullptr);
			inline Float4SpinBox(const Nz::Vector4f& value, LabelMode labelMode, QBoxLayout::Direction dir, QWidget* parent = nullptr);
			~Float4SpinBox() = default;

			void setValue(const Nz::Vector4f& value);
			Nz::Vector4f value();

		signals:
			void valueChanged(Nz::Vector4f newValue);

		private:
			void onSpinBoxValueChanged();

			QDoubleSpinBox* m_xSpinbox;
			QDoubleSpinBox* m_ySpinbox;
			QDoubleSpinBox* m_zSpinbox;
			QDoubleSpinBox* m_wSpinbox;
			bool m_ignoreSignal;
	};
}

#include <MapEditor/Widgets/Float4SpinBox.inl>

#endif
