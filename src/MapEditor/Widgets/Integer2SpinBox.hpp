// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_MAPEDITOR_WIDGETS_INTEGER2SPINBOX_HPP
#define BURGWAR_MAPEDITOR_WIDGETS_INTEGER2SPINBOX_HPP

#include <Nazara/Math/Vector2.hpp>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QWidget>

class QSpinBox;

namespace bw
{
	class Integer2SpinBox : public QWidget
	{
		Q_OBJECT

		public:
			enum class LabelMode
			{
				NoLabel,
				PositionLabel,
				SizeLabel
			};

			Integer2SpinBox(LabelMode labelMode, QBoxLayout::Direction dir, QWidget* parent = nullptr);
			inline Integer2SpinBox(const Nz::Vector2i64& value, LabelMode labelMode, QBoxLayout::Direction dir, QWidget* parent = nullptr);
			~Integer2SpinBox() = default;

			void setValue(const Nz::Vector2i64& value);
			Nz::Vector2i64 value();

		signals:
			void valueChanged(Nz::Vector2i64 newValue);

		private:
			void onSpinBoxValueChanged();

			QSpinBox* m_xSpinbox;
			QSpinBox* m_ySpinbox;
	};
}

#include <MapEditor/Widgets/Integer2SpinBox.inl>

#endif
