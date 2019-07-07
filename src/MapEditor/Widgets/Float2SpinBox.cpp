// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar Map Editor" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Widgets/Float2SpinBox.hpp>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QLabel>
#include <limits>

namespace bw
{
	Float2SpinBox::Float2SpinBox(LabelMode labelMode, QBoxLayout::Direction dir, QWidget* parent) :
	QWidget(parent),
	m_ignoreSignal(false)
	{
		auto onValueChanged = [this](double) 
		{ 
			if (!m_ignoreSignal) 
				onSpinBoxValueChanged(); 
		};

		m_xSpinbox = new QDoubleSpinBox;
		m_xSpinbox->setRange(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
		connect(m_xSpinbox, qOverload<double>(&QDoubleSpinBox::valueChanged), onValueChanged);

		m_ySpinbox = new QDoubleSpinBox;
		m_ySpinbox->setRange(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
		connect(m_ySpinbox, qOverload<double>(&QDoubleSpinBox::valueChanged), onValueChanged);

		QLayout* layout;

		switch (labelMode)
		{
			case LabelMode::NoLabel:
				layout = new QBoxLayout(dir);
				layout->addWidget(m_xSpinbox);
				layout->addWidget(m_ySpinbox);
				break;

			case LabelMode::PositionLabel:
			case LabelMode::SizeLabel:
			{
				QString xLabel;
				QString yLabel;

				if (labelMode == LabelMode::SizeLabel)
				{
					xLabel = tr("Width");
					yLabel = tr("Height");
				}
				else
				{
					xLabel = "X";
					yLabel = "Y";
				}

				if (dir == QBoxLayout::TopToBottom)
				{
					QFormLayout* formLayout = new QFormLayout;
					formLayout->addRow(xLabel, m_xSpinbox);
					formLayout->addRow(yLabel, m_ySpinbox);

					layout = formLayout;
				}
				else
				{
					QBoxLayout* boxLayout = new QBoxLayout(dir);

					QFormLayout* xLayout = new QFormLayout;
					xLayout->addRow(xLabel, m_xSpinbox);
					boxLayout->addLayout(xLayout);

					QFormLayout* yLayout = new QFormLayout;
					yLayout->addRow(yLabel, m_ySpinbox);
					boxLayout->addLayout(yLayout);

					layout = boxLayout;
				}
				break;
			}

			default:
				break;
		}

		layout->setContentsMargins(0, 0, 0, 0);
		setLayout(layout);
	}

	void Float2SpinBox::setValue(const Nz::Vector2f& value)
	{
		m_ignoreSignal = true;

		m_xSpinbox->setValue(value.x);
		m_ySpinbox->setValue(value.y);

		m_ignoreSignal = false;

		onSpinBoxValueChanged();
	}

	Nz::Vector2f Float2SpinBox::value()
	{
		Nz::Vector2f vec;
		vec.x = m_xSpinbox->value();
		vec.y = m_ySpinbox->value();

		return vec;
	}

	void Float2SpinBox::onSpinBoxValueChanged()
	{
		emit valueChanged(value());
	}
}

#include "Float2SpinBox.moc"
