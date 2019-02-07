// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Widgets/PositionEditWidget.hpp>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QHBoxLayout>
#include <limits>

namespace bw
{
	PositionEditWidget::PositionEditWidget(QWidget* parent) :
	QWidget(parent)
	{
		auto onValueChanged = [this](double) { onSpinBoxValueChanged(); };

		m_xSpinbox = new QDoubleSpinBox;
		m_xSpinbox->setRange(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
		connect(m_xSpinbox, qOverload<double>(&QDoubleSpinBox::valueChanged), onValueChanged);

		m_ySpinbox = new QDoubleSpinBox;
		m_ySpinbox->setRange(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
		connect(m_ySpinbox, qOverload<double>(&QDoubleSpinBox::valueChanged), onValueChanged);

		QHBoxLayout* layout = new QHBoxLayout;
		layout->addWidget(m_xSpinbox);
		layout->addWidget(m_ySpinbox);

		layout->setContentsMargins(0, 0, 0, 0);
		setLayout(layout);
	}

	void PositionEditWidget::setValue(const Nz::Vector2d & value)
	{
		m_xSpinbox->setValue(value.x);
		m_ySpinbox->setValue(value.y);
	}

	Nz::Vector2d PositionEditWidget::value()
	{
		Nz::Vector2d vec;
		vec.x = m_xSpinbox->value();
		vec.y = m_ySpinbox->value();

		return vec;
	}

	void PositionEditWidget::onSpinBoxValueChanged()
	{
		emit valueChanged(value());
	}
}

#include "PositionEditWidget.moc"
